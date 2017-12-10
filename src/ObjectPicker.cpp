#include <assert.h>
#include <list>
#include <iterator>
#include "LinearDisplacement.hpp"
#include "ObjectPicker.hpp"

#include "parm.hpp"
extern varSet vaParm[24];
extern int maxParm;

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
    xrayFeaturesDetector(size / 2, tmin, tmax, colorSplitDetetionRay),
    canny(Canny(size, state, dump, minlength, tmin, tmax, sigma)),
    inbw(image<pixelf>(size)),
    lock()
{
  static float blurMin = 0, blurMax = 30, blurStep = 0.01;
  vaParm[maxParm++] = (varSet){ &blurMin, &blurMax, &blurStep, &sigma, "canny blur", FLOAT };

  static unsigned int dumpMin = 1, dumpMax = 30, dumpStep = 1;
  vaParm[maxParm++] = (varSet){ &dumpMin, &dumpMax, &dumpStep, &dump, "canny dump", UINT };

  static unsigned int lMin = 1, lMax = 200, lStep = 1;
  vaParm[maxParm++] = (varSet){ &lMin, &lMax, &lStep, &minlength, "min length", UINT };
}

ObjectPicker::~ObjectPicker() {
  delete(state);
}

void *		ObjectPicker::detect(image<pixel16> * img) {

  static float	lastsigma;
  if (lastsigma != sigma) {
    lastsigma = sigma;
    canny.setBlur(sigma);
  }

  assert(img != NULL && img->pixel != NULL);
  assert(img->size == size);
  
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
  objectFeatures objectFeatures = detectFeatures(scany, img);
  objectEdges edges = findEdges(objectFeatures, img, dump);

  render(img, edges);
  optimizeDetection(lastObjectPosition);

  /*
  for (std::list<Canny::edge>::const_iterator i = edges->begin(); i != edges->end(); ++i) {
    img->pixel[(*i).position].setrvb(0, 255, 0);
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin(); j != (*i).point->end(); ++j) {
    	img->pixel[(*j).position].setrvb(255, 255, 70);
    }
  }
  */

  // detect features
  // detect edges: aabb search, edge caracterisation filter, inner/outer edge determination
  // filter edges and close undetected region
  // render object border and internal hole with blur effect
  // extract polygones.
  return NULL;
}

ObjectPicker::objectFeatures	ObjectPicker::detectFeatures(image<pixelf> * scany,
							     image<pixel16> * img) {
  objectFeatures	features;

  features.xraySplits = xrayFeaturesDetector.detect(scany, img);
  return features;
}

ObjectPicker::objectEdges	ObjectPicker::findEdges(objectFeatures objectFeatures,
							image<pixel16> * img,
							unsigned int mydump) {
  objectEdges	edges;
  Canny::edge	newedge;
  vec2		newPosition = 0;
  int		nb = 0;

  canny.clearState();
  for (unsigned int i = 0; i < 1 && objectFeatures.xraySplits.all.size() > 0; i++) {
    XrayFeatures::colorSplit split = objectFeatures.xraySplits.all.front();
    for (std::list<XrayFeatures::splitInfo *>::const_iterator j = split.split.begin();
	 j != split.split.end(); ++j) {
      if (canny.getEdge(newedge, (*j)->pos.to1D(img->size.x), mydump)
	  && newedge.length > minlength) {
	newPosition += (*j)->pos;
	edges.outerEdges.push_front(newedge); 
	nb++;
      }
    }
    objectFeatures.xraySplits.all.pop_front();
  }

  if (nb > 0) {
    lastObjectPosition = newPosition / nb;
  }
  return edges;
}

void				ObjectPicker::render(image<pixel16> * img, objectEdges edges) {
  int g = 0;
  // outer
  for (std::list<Canny::edge>::const_iterator i = edges.outerEdges.begin();
       i != edges.outerEdges.end(); ++i) {
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin();
	 j != (*i).point->end(); ++j) {
      img->pixel[(*j).position].setrvb(120, 10, 120);
    }
    img->pixel[(*i).position].setrvb(0, 255, 0);
    g++;
  }
  // inner
  for (std::list<Canny::edge>::const_iterator i = edges.innerEdges.begin();
       i != edges.innerEdges.end(); ++i) {
    img->pixel[(*i).position].setrvb(0, 255, 0);
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin();
	 j != (*i).point->end(); ++j) {
      img->pixel[(*j).position].setrvb(0, 0, 255);
    }
  }  
}

void				ObjectPicker::optimizeDetection(vec2 lastObjectPosition) {
  xrayFeaturesDetector.aimTarget(lastObjectPosition);
}

bool	ObjectPicker::setLock(void * l) {
  objectFeatures newlock = *(ObjectPicker::objectFeatures *)l;

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
