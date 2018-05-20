#include <assert.h>
#include <stack>
#include <list>
#include <iterator>
#include "XrayFeatures.hpp"
#include "vec2.hpp"
#include "vec2f.hpp"
#include "image.hpp"
#include "pixelf.hpp"
#include "pixel.hpp"
#include "LinearDisplacement.hpp"

#include "visualdebug.hpp"

extern unsigned int renderMode;

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
    maxBackGThreshold(28),
    objCMultiRayBonus(0.25),
    minRayPerObjC(1),
    maxObjCThreshold(15),
    endSplitMalus(1.2)
  {
  assert(xrayaimwidth > 4);
  PARMVSVAR(1, 100, 1, &rayCount, "rayCount");
  //PARMVSVAR(1, 200, 1, &rayAimWidth, "aim width", UINT);
  PARMVSVAR(10, 3000, 1, &aimPosition.x, (char*)"aim position X");
  PARMVSVAR(10, 3000, 1, &aimPosition.y, (char*)"aim position Y");
  PARMVSVAR(1, 300, 1, &aimRatio[0], (char*)"aim ratio target");
  PARMVSVAR(1, 300, 1, &aimRatio[1], (char*)"aim ratio lastpos");
  PARMVSVAR(1, 300, 1, &aimRatio[2], (char*)"aim ratio originpos");
  PARMVSVAR(0, 300, 1, &tmax, (char*)"Xray max");
  PARMVSVAR(0, 300, 1, &tmin, (char*)"Xray min");
  PARMVSVAR(1, 90, 0.1, &maxDiff, (char*)"Xray diff");
  PARMVSVAR(1, 90, 0.2, &maxBackGThreshold, (char*)"maxBackGThreshold");
  PARMVSVAR(1, 90, 0.2, &maxObjCThreshold, (char*)"maxObjCThreshold");
  PARMVSVAR(1, 2, 0.1, &endSplitMalus, (char*)"EndSpMalus");
}

XrayFeatures::~XrayFeatures()
{
}

XrayFeatures::xrayFeatures const &	XrayFeatures::detect(image<pixelf> * scany,
							     image<pixel> * img) {
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

void	XrayFeatures::extractFeatures(std::list<colorSplit> & xraySplit, image<pixel> * img) {
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
}

std::list<XrayFeatures::colorSplit>::iterator		XrayFeatures::searchBackGroundColors(std::list<colorSplit> & xraySplit,
											     image<pixel> * img) {
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
											 image<pixel> * img,
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
  
  features.edgesAABB[0] = vec2(INT_MAX, INT_MAX);
  features.edgesAABB[1] = vec2(INT_MIN, INT_MIN);
  for (std::list<colorSplit>::iterator j = objSplit.begin(); j != lastBestObjectSplit; j++) {
    for (std::list<splitInfo>::iterator k = (*j).split.begin(); k != (*j).split.end(); k++) {
      // (start + end) / 2
      //if ((aimPosition - (*i).pos).length * ((*i).start ? 1 : endSplitMalus) > )
      features.edges.push_front({ (*k).pos, (*k).start });
      if (features.edgesAABB[0].x > (*k).pos.x) {
	features.edgesAABB[0].x = (*k).pos.x;
      } else if (features.edgesAABB[0].y > (*k).pos.y) {
	features.edgesAABB[0].y = (*k).pos.y;
      }
      if (features.edgesAABB[1].x < (*k).pos.x) {
	features.edgesAABB[1].x = (*k).pos.x;
      } else if (features.edgesAABB[1].y < (*k).pos.y) {
	features.edgesAABB[1].y = (*k).pos.y;
      }      
    }
  }
}

std::list<XrayFeatures::colorSplit>::iterator		XrayFeatures::splitScoreThresholdSelection(std::list<colorSplit> & splits,
										   std::list<pixel> & colorOutput,
										   float threshold) {
  std::list<colorSplit>::iterator i;
  float	score;
  float count = 1;
  float scoreDiff;
  double scoreSum;

  splits.sort([](const colorSplit & a, const colorSplit & b) {
      return (a.score > b.score);
    });

  i = splits.begin();
  if (i == splits.end())
    return i;
  scoreSum = (*i).score;
  score = scoreSum;
  while ((*i).score != 0 &&
	 (scoreDiff = ((score - (float)(*i).score) / score * 100.0f)) < threshold) {
    colorOutput.push_front((*i++).color);
    if (i != splits.end()) {
      break;
    }
    scoreSum += (*i).score;
    score = scoreSum / ++count;
  }
  //printf("th %f\n", ((score - (float)(*i).score) / score * 100.0f));
  return i;
}

void		XrayFeatures::aimTarget(vec2 aimTargetPosition) {
  //(void)aimTargetPosition;
  aimPosition = (aimTargetPosition * aimRatio[0] +
		 aimPosition * aimRatio[1] +
		 originalAimPosition * aimRatio[2]
		 ) / (aimRatio[0] + aimRatio[1] + aimRatio[2]);
}

void		XrayFeatures::detectColorSplit(image<pixelf> * scany,
					       image<pixel> * img,
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
      if (renderMode == 5) {
	img->pixel[pos.to1D(img->size.x)].set((uint32_t)((uint32_t)16581375 / 8 * nbSplit) ^ 1);
      }
    }
    // search colorSplit groupe or create new one
    if (splitLength > 0) {
      split.start = false;
      split.length = splitLength;
      split.color.setrvb((uint8_t)(colorSum[0] / splitLength),
			 (uint8_t)(colorSum[1] / splitLength),
			 (uint8_t)(colorSum[2] / splitLength));
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
  (*bestSplit).color.setrvb((uint8_t)((*bestSplit).colorSum[0] / (*bestSplit).length),
			    (uint8_t)((*bestSplit).colorSum[1] / (*bestSplit).length),
			    (uint8_t)((*bestSplit).colorSum[2] / (*bestSplit).length));
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
      (*bestSplit).color.setrvb((uint8_t)((*bestSplit).colorSum[0] / (*bestSplit).length),
				(uint8_t)((*bestSplit).colorSum[1] / (*bestSplit).length),
				(uint8_t)((*bestSplit).colorSum[2] / (*bestSplit).length));
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
