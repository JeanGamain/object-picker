#include <assert.h>
#include <list>
#include <iterator>
#include "LinearDisplacement.hpp"
#include "ObjectPicker.hpp"

extern float diffa;
extern float diffb;
extern float diffc;
extern float diffd;

ObjectPicker::ObjectPicker(vec2 size)
  : truesize(size),
    resize(0.0f),
    size((resize > 0.0f) ? size * vec2(resize, resize) : size),
    state(new unsigned char[size.x * size.y]),
    dump(1),
    minlength(1), //10
    tmin(50),
    tmax(70),
    sigma(1.5f),
    colorSplitDetetionRay(17),
    maxPixelDiff(30.0f),
    canny(Canny(size, state, dump, minlength, tmin, tmax, sigma)),
    inbw(image<pixelf>(size)),
    lock(),
    aimPosition(size / 2)
{
}

ObjectPicker::~ObjectPicker() {
  delete(state);
}

void *		ObjectPicker::detect(image<pixel16> * img) {
  static float lastdump = diffa;
  static float lasttmin = diffb;
  static float lasttmax = diffc;
  static float lastsigma = diffd;
 
  assert(img != NULL && img->pixel != NULL);
  assert(img->size == size);

  if (lastdump != diffa) {
    lastdump = diffa;
    canny.setDump(dump + lastdump);
  }
  
  if (lasttmin != diffb) {
    lasttmin = diffb;
    canny.setMin(tmin + lasttmin);
  }

  if (lasttmax != diffc) {
    lasttmax = diffc;
    canny.setMax(tmax + lasttmax);
  }

  if (lastsigma != diffd) {
    lastsigma = diffd;
    //maxPixelDiff = 30.0f + lastsigma;
    canny.setBlur(sigma + lastsigma);
  }
  
  #pragma omp for
  for (int x = 0; x < (img->size.x * img->size.y); x++) {
      inbw.pixel[x].set(img->pixel[x].get());
  }
  /*
  if (resize > 0) {
    in->resize(newin, resize, resize);
    newin = G;
    }*/
  
  image<pixelf> * scany = canny.scan(&inbw);

  // if lock use lock
  objectFeature objFeature = detectCenterObjectFeature(scany, img);

  std::list<Canny::edge> * edges/* = canny.get()*/;

  canny.clearState();
  std::list<Canny::edge>	objectEdges;
  Canny::edge			newedge;

  vec2 newpos;
  int nb = 0;
  for (int i = 0; i < (dump + lastdump) && objFeature.xraySplits.all.size() > 0; i++) {
    colorSplit split = objFeature.xraySplits.all.front();
    for (std::list<splitInfo>::const_iterator j = split.mainSplit.begin();
	 j != split.mainSplit.end(); ++j) {
      if (canny.getEdge(newedge, (*j).start.to1D(img->size.x), dump + lastdump)
	  && newedge.length > minlength) {
	objectEdges.push_front(newedge); 
	newpos += (*j).start;
	nb++;
      }
    }
    objFeature.xraySplits.all.pop_front();
  }

  if (nb) {
    //aimPosition = ((newpos / j) * 50 + aimPosition * 50) / 100;
    aimPosition = ((newpos / nb) * 25 + size / 2 * 75) / 100;
  }
  /*
  for (std::list<Canny::edge>::const_iterator i = edges->begin(); i != edges->end(); ++i) {
    img->pixel[(*i).position].setrvb(0, 255, 0);
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin(); j != (*i).point->end(); ++j) {
    	img->pixel[(*j).position].setrvb(255, 255, 70);
    }
  }
  */
  
  edges = &objectEdges;
  
  for (std::list<Canny::edge>::const_iterator i = edges->begin(); i != edges->end(); ++i) {
    img->pixel[(*i).position].setrvb(0, 255, 0);
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin(); j != (*i).point->end(); ++j) {
	img->pixel[(*j).position].setrvb(255, 0, 0);
    }
  }
  return NULL;
}

void	ObjectPicker::concatColorSplit(colorSplitUnion * splits,
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

void	ObjectPicker::finalizeColorSplitUnion(colorSplitUnion * splits, colorSplitUnion * lastSplits) {
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

void		ObjectPicker::detectColorSplitFeature(image<pixelf> * scany,
						      image<pixel16> * img,
						      LinearDisplacement & line,
						      colorSplitUnion * lastSplit) {
  colorSplitUnion	splits;
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

ObjectPicker::objectFeature const &	ObjectPicker::detectCenterObjectFeature(image<pixelf> * scany, image<pixel16> * img) {
  static objectFeature	objectFeatures;
  const vec2	rayon(5, 0); 
  vec2		vector;
  
  objectFeatures.xraySplits.all.clear();
  for (int i = 0; i < colorSplitDetetionRay; i++) {
    vector = rayon;
    vector.rotate(360.0f / float(colorSplitDetetionRay) * i);
    LinearDisplacement line(aimPosition + vector, vector * img->size.y);
    detectColorSplitFeature(scany, img, line, &objectFeatures.xraySplits);
  }
  objectFeatures.xraySplits.all.sort([](const colorSplit & a, const colorSplit & b) {
      return (a.length > b.length);
    });
  return objectFeatures;
}

bool	ObjectPicker::setLock(void * l) {
  objectFeature newlock = *(ObjectPicker::objectFeature *)l;

  if (1) {
    lock = newlock;
    return true;
  }
  return false;
}

void * ObjectPicker::getLock() const {
  return (void *)&lock;
}

float	ObjectPicker::getResize() const {
  return resize;
}

void	ObjectPicker::setResize(float r) {
  if (r <= 0) {
    size = truesize;
  } else {
    size = vec2(cordinate(r * truesize.x), cordinate(r * truesize.y));
  }
  // resize canny
  resize = r;
}
