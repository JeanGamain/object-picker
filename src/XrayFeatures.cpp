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
  
  features.all.clear();
  for (unsigned int i = 0; i < rayCount; i++) {
    vector = baseRayVector;
    vector.rotate(360.0f / float(rayCount) * i);
    LinearDisplacement line(aimPosition + vector, vector * img->size.y);
    detectColorSplitFeatures(scany, img, line, &features);
  }
  features.all.sort([](const colorSplit & a, const colorSplit & b) {
      return (a.length > b.length);
    });
  return features;
}

void		XrayFeatures::aimTarget(vec2 aimTargetPosition) {
  //aimPosition = ((newpos / nb) * 10 + aimPosition * 90) / 100;
  aimPosition = (aimTargetPosition * 27 +
		 aimPosition * 50 +
		 originalAimPosition * 23
		 ) / 100;
}
void		XrayFeatures::detectColorSplitFeatures(image<pixelf> * scany,
						       image<pixel16> * img,
						       LinearDisplacement & line,
						       xrayFeatures * lastSplit) {
  xrayFeatures	splits;
  splitInfo	split;
  pixel16	splitColor;
  pixel16	lastSplitColor = 0;
  unsigned int	colorSum[3];
  unsigned int	splitLength;

  int nbSplit = 0; //

  // check first case
  for (vec2 pos = line.get();
       pos > vec2(0, 0) && pos < scany->size && !line.end();) {
    split.start = pos;
    split.sideEdgeColor = lastSplitColor;
    while (pos > vec2(0, 0) && pos < scany->size && !line.end() // jump edge
	   && scany->pixel[pos.to1D(img->size.x)] >= tmax)
      pos = line.get();
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
      splitColor.setrvb((uint16_t)(colorSum[0] / splitLength),
			(uint16_t)(colorSum[1] / splitLength),
			(uint16_t)(colorSum[2] / splitLength));
      split.end = pos;
      concatColorSplit(&splits, split, splitColor, splitLength, colorSum);
      lastSplitColor = splitColor;
      nbSplit++;
    }
  }
  finalizeColorSplitUnion(&splits, lastSplit);
}

void	XrayFeatures::concatColorSplit(xrayFeatures * splits,
				       splitInfo split,
				       pixel16	splitColor,
				       unsigned int splitLength,
				       unsigned int colorSum[3]) {
  std::list<colorSplit>::iterator i;
  for (i = splits->all.begin(); i != splits->all.end(); ++i) {
    if ((*i).color.diff(splitColor) < 3) {
      (*i).colorSum[0] += colorSum[0];
      (*i).colorSum[1] += colorSum[1];
      (*i).colorSum[2] += colorSum[2];
      (*i).length += splitLength;
      (*i).color.setrvb((uint16_t)((*i).colorSum[0] / (*i).length),
			(uint16_t)((*i).colorSum[1] / (*i).length),
			(uint16_t)((*i).colorSum[2] / (*i).length));
      (*i).subSection.push_front(split);
      break;
    }
  }
  if (i == splits->all.end()) {
    colorSplit newsplit;
    newsplit.subSection.push_front(split);
    newsplit.colorSum[0] = colorSum[0];
    newsplit.colorSum[1] = colorSum[1];
    newsplit.colorSum[2] = colorSum[2];
    newsplit.length = splitLength;
    newsplit.color = splitColor;	
    splits->all.push_front(newsplit);
  }
}

void	XrayFeatures::finalizeColorSplitUnion(xrayFeatures * splits,
					      xrayFeatures * lastSplits) {
  for (std::list<colorSplit>::iterator i = splits->all.begin(); i != splits->all.end(); ++i) {
    std::list<colorSplit>::iterator j;
    for (j = lastSplits->all.begin();
	 j != lastSplits->all.end() && (*i).color.diff((*j).color) > 3;
	 ++j);
    if (j == lastSplits->all.end()) { //push on empty list them concat after search
      (*i).mainSplit.push_front((*i).subSection.front());
      (*i).subSection.pop_front();
      lastSplits->all.push_front((*i));
    } else {
      (*j).colorSum[0] += (*i).colorSum[0];
      (*j).colorSum[1] += (*i).colorSum[1];
      (*j).colorSum[2] += (*i).colorSum[2];
      (*j).length += (*i).length;
      (*j).color.setrvb((uint16_t)((*j).colorSum[0] / (*j).length),
			(uint16_t)((*j).colorSum[1] / (*j).length),
			(uint16_t)((*j).colorSum[2] / (*j).length));
      (*j).mainSplit.push_front((*i).subSection.front());
      (*j).subSection.splice((*j).subSection.begin(), (*i).subSection, ++(*i).subSection.begin(), (*i).subSection.end());
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
