#include <assert.h>
#include <list>
#include <iterator>
#include "XrayFeatures.hpp"
#include "vec2.hpp"
#include "image.hpp"
#include "pixelf.hpp"
#include "pixel16.hpp"
#include "LinearDisplacement.hpp"

XrayFeatures::XrayFeatures(vec2 aimTargetPositon,
			   float min, float max,
			   unsigned int rayc,
			   unsigned int xrayaimwidth)
  : rayCount(rayc),
    rayAimWidth(xrayaimwidth),
    baseRayVector(vec2(xrayaimwidth, 0)),
    aimPosition(aimTargetPositon),
    originalAimPosition(aimTargetPositon),
    tmin(min),
    tmax(max)
{
  assert(xrayaimwidth > 4);
}

XrayFeatures::~XrayFeatures()
{
}

XrayFeatures::xrayFeatures const &	XrayFeatures::detect(image<pixelf> * scany,
							     image<pixel16> * img) {
  vec2		vector;

  for (std::list<colorSplit>::iterator i = features.all.begin();
       i != features.all.end();
       i++) {
    for (std::list<splitInfo*>::iterator j = (*i).split.begin();
	 j != (*i).split.end();
	 j++) {
      free((*j));
    }
    (*i).split.clear();
  }
  features.all.clear();
  for (unsigned int i = 0; i < rayCount; i++) {
    vector = baseRayVector;
    vector.rotate(360.0f / float(rayCount) * i);
    LinearDisplacement line(aimPosition + vector, vector * img->size.y);
    detectColorSplitFeatures(scany, img, line, &features);
  }
  extractFeatures();
  return features;
}

void	XrayFeatures::extractFeatures() {
  for (std::list<colorSplit>::iterator i = features.all.begin();
       i != features.all.end();
       i++) {
    (*i).score = (*i).length;
  }
  features.all.sort([](const colorSplit & a, const colorSplit & b) {
      return (a.score > b.score);
    });
}


void		XrayFeatures::aimTarget(vec2 aimTargetPosition) {
  //aimPosition = ((newpos / nb) * 10 + aimPosition * 90) / 100;
  /*  aimPosition = (aimTargetPosition * 27 +
		 aimPosition * 50 +
		 originalAimPosition * 23
		 ) / 100;*/
}

void		XrayFeatures::detectColorSplitFeatures(image<pixelf> * scany,
						       image<pixel16> * img,
						       LinearDisplacement & line,
						       xrayFeatures * lastSplit) {
  xrayFeatures	splits;
  splitInfo *	split;
  vec2		pos = line.get();
  vec2		truePos;
  splitInfo *	lastNewSplit = NULL;
  unsigned int	colorSum[3];
  unsigned int	splitLength;
  int		i;
  int nbSplit = 0; //

  // go to first edge
  while (pos > vec2(0, 0) && pos < scany->size && !line.end()) {
    truePos = pos;
    i = 1;
    do {
      pos = line.get();
      truePos += pos;
      i++;
    } while (pos > vec2(0, 0) && pos < scany->size && !line.end() // jump edge
	     && scany->pixel[pos.to1D(img->size.x)] >= tmax); 
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
      split = (splitInfo*)malloc(sizeof(splitInfo));
      split->length = splitLength;
      split->color.setrvb((uint16_t)(colorSum[0] / splitLength),
			  (uint16_t)(colorSum[1] / splitLength),
			  (uint16_t)(colorSum[2] / splitLength));
      if (lastNewSplit != NULL) {
	lastNewSplit->next = split;
        split->prev = lastNewSplit;
      } else {
	split->prev = NULL;
      }
      lastNewSplit = split;
      split->next = NULL;
      split->pos = truePos;
      concatColorSplit(&splits, split, colorSum);
      nbSplit++;
    }
  }
  finalizeColorSplitUnion(&splits, lastSplit);
}

void	XrayFeatures::concatColorSplit(xrayFeatures * splits,
				       splitInfo * split,
				       unsigned int colorSum[3]) {
  std::list<colorSplit>::iterator bestSplit = splits->all.end();
  float	bestDiff = 6;
  float	diff;
  
  for (std::list<colorSplit>::iterator i = splits->all.begin();
       i != splits->all.end(); ++i) {
    diff = (*i).color.diff(split->color);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestSplit = i;
    }
  }
  if (bestSplit == splits->all.end()) { // new
    colorSplit newsplit;
    newsplit.split.push_front(split);
    newsplit.colorSum[0] = colorSum[0];
    newsplit.colorSum[1] = colorSum[1];
    newsplit.colorSum[2] = colorSum[2];
    newsplit.length = split->length;
    newsplit.color = split->color;	
    splits->all.push_front(newsplit);
  } else { // concat
    (*bestSplit).colorSum[0] += colorSum[0];
    (*bestSplit).colorSum[1] += colorSum[1];
    (*bestSplit).colorSum[2] += colorSum[2];
    (*bestSplit).length += split->length;
    (*bestSplit).color.setrvb((uint16_t)((*bestSplit).colorSum[0] / (*bestSplit).length),
			      (uint16_t)((*bestSplit).colorSum[1] / (*bestSplit).length),
			      (uint16_t)((*bestSplit).colorSum[2] / (*bestSplit).length));
    (*bestSplit).split.push_front(split);
  }
}

void	XrayFeatures::finalizeColorSplitUnion(xrayFeatures * splits,
					      xrayFeatures * lastSplits) {
  std::list<colorSplit>::iterator bestSplit;
  float	bestDiff = 6;
  float	diff;
 
  for (std::list<colorSplit>::iterator i = splits->all.begin();
       i != splits->all.end(); ++i) {
    bestSplit = lastSplits->all.end();
    for (std::list<colorSplit>::iterator j = lastSplits->all.begin();
	 j != lastSplits->all.end(); ++j) {
      diff = (*i).color.diff((*j).color);
      if (diff < bestDiff) {
	bestDiff = diff;
	bestSplit = j;
      }
    }
    if (bestSplit == lastSplits->all.end()) { //push on empty list them concat after search
      (*i).nbray = 1;
      lastSplits->all.push_front((*i));
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
