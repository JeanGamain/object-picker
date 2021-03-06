#include <assert.h>
#include <list>
#include <iterator>
#include "pixel.hpp"
#include "LinearDisplacement.hpp"
#include "ObjectPicker.hpp"

#include "visualdebug.hpp"

extern unsigned int renderMode;

ObjectPicker::ObjectPicker(vec2 size)
  : truesize(size),
    resize(0.0f),
    size((resize > 0.0f) ? size * vec2(resize, resize) : size),
    state(new unsigned char[size.x * size.y]),
    dump(1),
    minlength(28),
    tmin(50),
    tmax(70),
    sigma(1.5f),
    colorSplitDetetionRay(39),
    maxPixelDiff(10.0f),
    xrayFeaturesDetector(size / 2, tmin, tmax, colorSplitDetetionRay),
    canny(Canny(size, state, dump, minlength, tmin, tmax, sigma)),
    greyScaleImg(image<pixelf>(size)),
    blurGreyScaleImg(image<pixelf>(size)),
    lock()
{
  PARMVSVAR(0, 30, 0.01, &sigma, "canny blur");
  PARMVSVAR(1, 30, 1, &dump, "canny dump");
  PARMVSVAR(1, 200, 1, &minlength, "min length");
  PARMVSVAR(0, 40, 1, &maxPixelDiff, "canny maxPixelDiff");
}

ObjectPicker::~ObjectPicker() {
  delete(state);
}

void *		ObjectPicker::detect(image<pixel> & img) {
  assert(img.pixel != NULL);
  assert(img.size == size);

  static float lastsigma = sigma;
  if (lastsigma != sigma) {
    lastsigma = sigma;
    canny.setBlur(sigma);
  }
  
  img.getGreyScale(greyScaleImg);
  
  
  if (renderMode == 2) {
    img.set(greyScaleImg);
    return NULL;
  }
  
  
  /*
  if (resize > 0) {
    in->resize(newin, resize, resize);
    newin = G;
    }*/
  
  image<pixelf> * scany = canny.scan(greyScaleImg);
  if (scany == NULL) {
    return NULL;
  }
  
  if (renderMode == 3 || renderMode == 4) {
    for (int x = 0; x < (img.size.x * img.size.y); x++) {
      img.pixel[x].set((uint8_t)(ABS(scany->pixel[x].get()) / 4));
    }
    return NULL;
  }
  
  // if lock use lock
  objectFeatures objectFeatures = detectFeatures(scany, &img);
  objectEdges edges = findEdges(objectFeatures, &img, dump);
  
  //render(img, edges);
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
							     image<pixel> * img) {
  objectFeatures	features;

  features.xray = xrayFeaturesDetector.detect(scany, img);
  return features;
}

ObjectPicker::objectEdges	ObjectPicker::findEdges(const objectFeatures & features,
							image<pixel> * img,
							unsigned int mydump) {
  objectEdges	edges;
  Canny::edge	newedge;
  vec2		newPosition = cordinate(0);
  vec2		pos;
  int		nb = 0;

  
  canny.clearState();
  if (renderMode != 6 && renderMode != 7) {
    for (std::list<XrayFeatures::edgePoint2d>::const_iterator i = features.xray.edges.begin();
	 i != features.xray.edges.end(); i++) {
      if (canny.getEdge(newedge, (*i).position.to1D(img->size.x), mydump, features.xray, *img, maxPixelDiff, (*i).normal)
	  && newedge.length > minlength) {
      newPosition += (*i).position;
      edges.outerEdges.push_front(newedge);
      nb++;
      }
    }
  } else {
    for (cordinate x = 2; x < (img->size.x - 2); x++) {
      for (cordinate y = 2; y < (img->size.y - 2); y++) {
	if (canny.getEdge(newedge, y * img->size.x + x , mydump, features.xray, *img, maxPixelDiff, 0)
	    && newedge.length > minlength) {
	  edges.outerEdges.push_front(newedge);
	}
      }
    }
  }
  /*
  for (pos.x = features.xray.edgesAABB[0].x; pos.x <= features.xray.edgesAABB[1].x; pos.x += 2) {
    for (pos.y = features.xray.edgesAABB[0].y; pos.y <= features.xray.edgesAABB[1].y; pos.y += 2) {
      if (canny.getEdge(newedge, pos.to1D(img->size.x), mydump, features, img, maxPixelDiff, )
	  && newedge.length > minlength) {
	edges.innerEdges.push_front(newedge);
      }
    }
    }*/
  if (nb > 0) {
    lastObjectPosition = newPosition / nb;
  }
  return edges;
}

void				ObjectPicker::render(image<pixel> * img, objectEdges edges) {
  int g = 0;
  // outer
  for (std::list<Canny::edge>::const_iterator i = edges.outerEdges.begin();
       i != edges.outerEdges.end(); ++i) {
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin();
	 j != (*i).point->end(); ++j) {
      //uint8_t r = 220 / edges.outerEdges.size() * g + 25;
      //img->pixel[(*j).position].setrvb(r % 5 * (255 / 5), r % 6 * (255 / 6), r % 7 * (255 / 7));
      img->pixel[(*j).position].setrvb(0, 0, 255);
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
