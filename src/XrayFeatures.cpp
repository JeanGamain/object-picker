#include <assert.h>
#include <stack>
#include <list>
#include <iterator>
#include "XrayFeatures.hpp"
#include "vec2.hpp"
#include "vec2f.hpp"
#include "image.hpp"
#include "pixelf.hpp"
#include "pixel16.hpp"
#include "LinearDisplacement.hpp"

#include "parm.hpp"
extern varSet vaParm[24];
extern int maxParm;

XrayFeatures::XrayFeatures(vec2 aimTargetPositon,
			   float min, float max,
			   unsigned int rayc,
			   unsigned int xrayaimwidth)
  : rayCount(rayc),
    rayAimWidth(xrayaimwidth),
    aimRatio{28, 50, 4},
    baseRayVector(vec2f(xrayaimwidth, 0)),
    aimPosition(aimTargetPositon),
    originalAimPosition(aimTargetPositon),
    tmin(min),
    tmax(max),
    maxDiff(9),
    backGMultiRayBonus(0.25),
    minRayPerBackG(1), // 2 -> ?3
    maxBackGThreshold(17),
    objCMultiRayBonus(0.25),
    minRayPerObjC(1),
    maxObjCThreshold(15),
    endSplitMalus(1.2)
  {
  assert(xrayaimwidth > 4);
  static unsigned int MinRCount = 1, MaxRCount = 100, StepRCount = 1;
  vaParm[maxParm++] = (varSet){ &MinRCount, &MaxRCount, &StepRCount, &rayCount, "rayCount", UINT };
  //static unsigned int MinAimW = 1, MaxAimW = 200, StepAimW = 1;
  //vaParm[maxParm++] = (varSet){ &MinAimW, &MaxAimW, &StepAimW, &rayAimWidth, "aim width", UINT };
  static unsigned int MinPx = 10, MaxPx = 3000, StepPx = 1;
  vaParm[maxParm++] = (varSet){ &MinPx, &MaxPx, &StepPx, &aimPosition.x, "aim position X", UINT };
  static unsigned int MinPy = 10, MaxPy = 3000, StepPy = 1;
  vaParm[maxParm++] = (varSet){ &MinPy, &MaxPy, &StepPy, &aimPosition.y, "aim position Y", UINT };

  static unsigned int MinR1 = 1, MaxR1 = 300, StepR1 = 1;
  vaParm[maxParm++] = (varSet){ &MinR1, &MaxR1, &StepR1, &aimRatio[0], "aim ratio target", UINT };
  static unsigned int MinR2 = 1, MaxR2 = 300, StepR2 = 1;
  vaParm[maxParm++] = (varSet){ &MinR2, &MaxR2, &StepR2, &aimRatio[1], "aim ratio lastpos", UINT };
  static unsigned int MinR3 = 1, MaxR3 = 300, StepR3 = 1;
  vaParm[maxParm++] = (varSet){ &MinR3, &MaxR3, &StepR3, &aimRatio[2], "aim ratio originpos", UINT };

  static float MinMax = 0, MaxMax = 300, StepMax = 1;
  vaParm[maxParm++] = (varSet){ &MinMax, &MaxMax, &StepMax, &tmax, "Xray max", FLOAT };
  static float MinMin = 0, MaxMin = 300, StepMin = 1;
  vaParm[maxParm++] = (varSet){ &MinMin, &MaxMin, &StepMin, &tmin, "Xray min", FLOAT };

  static float MinDiff = 1, MaxDiff = 90, StepDiff = 0.1;
  vaParm[maxParm++] = (varSet){ &MinDiff, &MaxDiff, &StepDiff, &maxDiff, "Xray diff", FLOAT };
}

XrayFeatures::~XrayFeatures()
{
}

XrayFeatures::xrayFeatures const &	XrayFeatures::detect(image<pixelf> * scany,
							     image<pixel16> * img) {
  std::list<colorSplit>			xraySplit;
  std::list<colorSplit>			raySplit[rayCount];

  for (unsigned int i = 0; i < rayCount; i++) {
    vec2f vector = baseRayVector;
    vector.rotate(360.0f / float(rayCount) * i);
    LinearDisplacement line(vec2f(aimPosition.x, aimPosition.y) + vector, vector * img->size.y);
    detectColorSplit(scany, img, line, raySplit[i], i);
  }
  for (unsigned int i = 0; i < rayCount; i++) {
    finalizeColorSplitUnion(raySplit[i], xraySplit);
  }
  extractFeatures(xraySplit, img);
  return features;
}

void	XrayFeatures::extractFeatures(std::list<colorSplit> & xraySplit, image<pixel16> * img) {
  // background color
  std::list<colorSplit>::iterator lastBestBackground =
    searchBackGroundColors(xraySplit, img);
  // object color
  std::list<colorSplit> objSplit;
  std::list<colorSplit>::iterator lastBestObjectSplit =
    searchObjectColors(xraySplit, img, objSplit, lastBestBackground);
  // object edges
  searchObjectEdges(objSplit, lastBestObjectSplit);

  //printf("size bg %lu, objc %lu, edges %lu\n", features.backgroundColor.size(), features.objectColor.size(), features.edges.size());
  // xray start from front edges
  // refonte image<x> * to &
  // refonte pixel16 pixel
  // refonte rgba32
}

std::list<XrayFeatures::colorSplit>::iterator		XrayFeatures::searchBackGroundColors(std::list<colorSplit> & xraySplit,
											     image<pixel16> * img) {
  features.backgroundColor.clear();
  for (std::list<colorSplit>::iterator i = xraySplit.begin(); i != xraySplit.end(); i++) {
    if ((*i).nbray >= minRayPerBackG)
      (*i).score = (*i).length + (img->size.x * backGMultiRayBonus * (*i).nbray);
    else
      (*i).score = 0;
  }  
  return splitScoreThresholdSelection(xraySplit, features.backgroundColor, maxBackGThreshold);
}

std::list<XrayFeatures::colorSplit>::iterator		XrayFeatures::searchObjectColors(std::list<colorSplit> & xraySplit,
											 image<pixel16> * img,
											 std::list<colorSplit> & objSplit,
											 std::list<colorSplit>::iterator & lastBestBackground) {
  unsigned int		colorSum[3];
  const cordinate	maxLength = (img->size / 2).length();
  unsigned int		sumDiCentering;
  
  features.objectColor.clear();
  for (std::list<colorSplit>::iterator j = xraySplit.begin(); j != lastBestBackground; j++) {
    for (std::list<splitInfo>::iterator k = (*j).split.begin(); k != (*j).split.end(); k++) { 
      if ((*k).next != NULL) {
	(*k).next->start = false;
	colorSum[0] = (unsigned int)(*k).next->color.getr(); // get comp
	colorSum[1] = (unsigned int)(*k).next->color.getv();
	colorSum[2] = (unsigned int)(*k).next->color.getb();
	concatColorSplit(objSplit, *(*k).next, colorSum);
      }
      if ((*k).prev != NULL) {
	(*k).prev->start = true;
	colorSum[0] = (unsigned int)(*k).prev->color.getr();
	colorSum[1] = (unsigned int)(*k).prev->color.getv();
	colorSum[2] = (unsigned int)(*k).prev->color.getb();
	concatColorSplit(objSplit, *(*k).prev, colorSum);
      }
    }
  }
  for (std::list<colorSplit>::iterator i = objSplit.begin(); i != objSplit.end(); i++) {
    bool uniqueRayId[rayCount] = { false };
    sumDiCentering = 0;
    for (std::list<splitInfo>::iterator k = (*i).split.begin(); k != (*i).split.end(); k++) { 
      uniqueRayId[(*k).rayId] = true;
      sumDiCentering += maxLength - (aimPosition - (*k).pos).length() * (((*k).start) ? 1 : endSplitMalus);
      // start + end / 2
    };
    (*i).nbray = 0;
    for (unsigned int rayI = 0; rayI < rayCount; rayI++)
      if (uniqueRayId[rayI])
	(*i).nbray++;
    //(end * r1 + start * r2 / (r1 * r2))
    // (*i).length
    if ((*i).nbray >= minRayPerObjC)
      (*i).score =
	(objCMultiRayBonus * (*i).nbray) + sumDiCentering;
    else
      (*i).score = 0;
  }
  return splitScoreThresholdSelection(objSplit, features.objectColor, maxObjCThreshold);
}

void			XrayFeatures::searchObjectEdges(std::list<colorSplit> & objSplit,
					  std::list<colorSplit>::iterator & lastBestObjectSplit) {
  features.edges.clear();
  for (std::list<colorSplit>::iterator j = objSplit.begin(); j != lastBestObjectSplit; j++) {
    for (std::list<splitInfo>::iterator k = (*j).split.begin(); k != (*j).split.end(); k++) {
      // (start + end) / 2
      //if ((aimPosition - (*i).pos).length * ((*i).start ? 1 : endSplitMalus) > )
      features.edges.push_front({ (*k).pos, (*k).start });
    }
  }
}

std::list<XrayFeatures::colorSplit>::iterator		XrayFeatures::splitScoreThresholdSelection(std::list<colorSplit> & splits,
										   std::list<pixel16> & colorOutput,
										   float threshold) {
  std::list<colorSplit>::iterator i;
  float	score;
  float count = 1;
  float scoreSum;

  splits.sort([](const colorSplit & a, const colorSplit & b) {
      return (a.score > b.score);
    });

  i = splits.begin();
  if (i == splits.end())
    return i;
  scoreSum = (*i).score;
  score = scoreSum;
  while (i != splits.end() && (*i).score != 0 &&
	 ((score - (float)(*i).score) / score * 100.0f) < threshold) {
    colorOutput.push_front((*i++).color);
    scoreSum += (*i).score;
    score = scoreSum / ++count;
  }
  //printf("pos %f\n", count);
  return i;
}

void		XrayFeatures::aimTarget(vec2 aimTargetPosition) {
  (void)aimTargetPosition;
  /*aimPosition = (aimTargetPosition * aimRatio[0] +
		 aimPosition * aimRatio[1] +
		 originalAimPosition * aimRatio[2]
		 ) / (aimRatio[0] + aimRatio[1] + aimRatio[2]);*/
}

void		XrayFeatures::detectColorSplit(image<pixelf> * scany,
					       image<pixel16> * img,
					       LinearDisplacement & line,
					       std::list<colorSplit> & splits,
					       unsigned int rayId) {
  vec2		pos = line.get();
  vec2		truePos;
  splitInfo	split;
  splitInfo *	newSplit;
  splitInfo *	lastNewSplit = NULL;
  unsigned int	colorSum[3];
  unsigned int	splitLength;
  int		i;
  int nbSplit = 0; //

  split.rayId = rayId;
  // go to first edge
  while (pos > vec2(0, 0) && pos < scany->size && !line.end()) {
    truePos = pos;
    i = 1;
    do {
      pos = line.get();
      truePos += pos;
      i++;
    } while (pos > vec2(0, 0) && pos < scany->size && !line.end() // jump edge
	     && scany->pixel[pos.to1D(img->size.x)] >= tmin); 
    truePos /= i;
    
    splitLength = 0;
    colorSum[0] = 0;
    colorSum[1] = 0;
    colorSum[2] = 0;
    for (;pos > vec2(0, 0) && pos < scany->size && !line.end()
	   && scany->pixel[pos.to1D(img->size.x)] < tmax;
	 pos = line.get()) {
      splitLength++;
      colorSum[0] += img->pixel[pos.to1D(img->size.x)].getr();
      colorSum[1] += img->pixel[pos.to1D(img->size.x)].getv();
      colorSum[2] += img->pixel[pos.to1D(img->size.x)].getb();  
      img->pixel[pos.to1D(img->size.x)].pixel = uint16_t(nbSplit * 65025 / 18); //
    }
    // search colorSplit groupe or create new one
    if (splitLength > 0) {
      split.start = false;
      split.length = splitLength;
      split.color.setrvb((uint16_t)(colorSum[0] / splitLength),
			 (uint16_t)(colorSum[1] / splitLength),
			 (uint16_t)(colorSum[2] / splitLength));
      split.prev = (lastNewSplit != NULL) ? lastNewSplit : NULL;
      split.next = NULL;
      split.pos = truePos;
      newSplit = concatColorSplit(splits, split, colorSum);
      if (lastNewSplit != NULL)
	lastNewSplit->next = newSplit;
      lastNewSplit = newSplit;
      nbSplit++;
    }
  }
}

XrayFeatures::splitInfo *		XrayFeatures::concatColorSplit(std::list<colorSplit> & splits,
								       splitInfo & split,
								       unsigned int colorSum[3]) {
  std::list<colorSplit>::iterator bestSplit = splits.end();
  float	bestDiff = maxDiff;
  float	diff;

  for (std::list<colorSplit>::iterator i = splits.begin();
       i != splits.end(); ++i) {
    diff = (*i).color.diff(split.color);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestSplit = i;
    }
  }
  if (bestSplit == splits.end()) { // new
    colorSplit newsplit;
    newsplit.split.push_front(split);
    newsplit.colorSum[0] = colorSum[0];
    newsplit.colorSum[1] = colorSum[1];
    newsplit.colorSum[2] = colorSum[2];
    newsplit.length = split.length;
    newsplit.color = split.color;
    splits.push_front(newsplit);
    return &(*(*splits.begin()).split.begin());
  }
  (*bestSplit).colorSum[0] += colorSum[0];
  (*bestSplit).colorSum[1] += colorSum[1];
  (*bestSplit).colorSum[2] += colorSum[2];
  (*bestSplit).length += split.length;
  (*bestSplit).color.setrvb((uint16_t)((*bestSplit).colorSum[0] / (*bestSplit).length),
			    (uint16_t)((*bestSplit).colorSum[1] / (*bestSplit).length),
			    (uint16_t)((*bestSplit).colorSum[2] / (*bestSplit).length));
  (*bestSplit).split.push_front(split);
  return &(*(*bestSplit).split.begin());
}

void	XrayFeatures::finalizeColorSplitUnion(std::list<colorSplit> & splits,
					      std::list<colorSplit> & lastSplits) {
  std::list<colorSplit>::iterator bestSplit;
  float	bestDiff = maxDiff;
  float	diff;
 
  for (std::list<colorSplit>::iterator i = splits.begin();
       i != splits.end(); ++i) {
    bestSplit = lastSplits.end();
    for (std::list<colorSplit>::iterator j = lastSplits.begin();
	 j != lastSplits.end(); ++j) {
      diff = (*i).color.diff((*j).color);
      if (diff < bestDiff) {
	bestDiff = diff;
	bestSplit = j;
      }
    }
    if (bestSplit == lastSplits.end()) {
      (*i).nbray = 1;
      lastSplits.push_front((*i));
    } else {
      (*bestSplit).nbray++;
      (*bestSplit).colorSum[0] += (*i).colorSum[0];
      (*bestSplit).colorSum[1] += (*i).colorSum[1];
      (*bestSplit).colorSum[2] += (*i).colorSum[2];
      (*bestSplit).length += (*i).length;
      (*bestSplit).color.setrvb((uint16_t)((*bestSplit).colorSum[0] / (*bestSplit).length),
				(uint16_t)((*bestSplit).colorSum[1] / (*bestSplit).length),
				(uint16_t)((*bestSplit).colorSum[2] / (*bestSplit).length));
      (*bestSplit).split.splice((*bestSplit).split.begin(),
				(*i).split, (*i).split.begin(), (*i).split.end());
    }
  }
}

void	XrayFeatures::setRayCount(unsigned int rayc) {
  rayCount = rayc;
}

void	XrayFeatures::setMin(float min) {
  tmin = min;
}

void	XrayFeatures::setMax(float max) {
  tmax = max;
}
