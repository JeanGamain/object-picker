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
    dump(10),
    minlength(1), //10
    tmin(50),
    tmax(70),
    sigma(1.5f),
    maxPixelDiff(30.0f),
    canny(new Canny(size, dump, minlength, tmin, tmax, sigma)),
    inbw(new image<pixelf>(size)),
    lock()
{

}

ObjectPicker::~ObjectPicker() {
  delete canny;
  delete inbw;
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
    canny->setDump(dump + lastdump);
  }
  
  if (lasttmin != diffb) {
    lasttmin = diffb;
    canny->setMinLength(minlength + lasttmin);
  }

  if (lasttmax != diffc) {
    lasttmax = diffc;
    canny->setMax(tmax + lasttmax);
  }

  if (lastsigma != diffd) {
    lastsigma = diffd;
    maxPixelDiff = 30.0f + lastsigma;
    //canny->setBlur(sigma + lastsigma);
  }
  
  #pragma omp for
  for (int x = 0; x < (img->size.x * img->size.y); x++) {
      inbw->pixel[x].set(img->pixel[x].get());
  }
  /*
  if (resize > 0) {
    in->resize(newin, resize, resize);
    newin = G;
    }*/
  
  image<pixelf> * scany = canny->scan(inbw);
  // if lock use lock
  objectFeature objFeature = detectCenterObjectFeature(scany, img);

  //printf("feature size %lu %lu\n", objFeature.sideEdgeColor[0].size(), objFeature.sideEdgeColor[1].size());

  std::list<Canny::edge> * edges = canny->get();

  for (std::list<Canny::edge>::const_iterator i = edges->begin();
       i != edges->end();
       ++i) {
    img->pixel[(*i).position].setrvb(0, 255, 0);
    for (std::list<Canny::edgePoint>::const_iterator j = (*i).point->begin();
	 j != (*i).point->end();
	 ++j) {
      if (isMatchingObjectFeatures(*img, objFeature, (*j).position, (*j).normal, maxPixelDiff))
	img->pixel[(*j).position].setrvb(0, 0, 255);
      else
	img->pixel[(*j).position].setrvb(255, 0, 0);
    }
  }
  return NULL;
}

bool	ObjectPicker::isMatchingObjectFeatures(image<pixel16> const & image, objectFeature const & objFeature, cordinate position, char normal, float maxDiff) {
  int d;
  pixel16 edgeColors[2];
 
  edgeColors[0] = image.pixel[position + dirNormal[0][(int)normal].to1D(image.size.x)];
  edgeColors[1] = image.pixel[position + dirNormal[1][(int)normal].to1D(image.size.x)];
  for (std::list<pixel16>::const_iterator k = objFeature.sideEdgeColor[0].begin();
       k != objFeature.sideEdgeColor[0].end();
       ++k) {
    d = -1;
    if (edgeColors[0].diff(*k) < maxDiff) {
      d = 1;
    } else if (edgeColors[1].diff(*k) < maxDiff) {
      d = 0;
    }
    if (d == -1)
      continue;
    for (std::list<pixel16>::const_iterator l = objFeature.sideEdgeColor[1].begin();
	 l != objFeature.sideEdgeColor[1].end();
	 ++l) {
      if (edgeColors[d].diff(*l) < maxDiff)
	return true;
    }
  }
  return false;
}

ObjectPicker::objectFeature const &	ObjectPicker::detectCenterObjectFeature(image<pixelf> * scany, image<pixel16> * img) {
  static objectFeature object;
  const vec2 rayon = vec2(10, 0); // pixel
  const vec2 center = scany->size / 2;
  const int ray = 9;
  vec2 pos, vector;
  colorSplit	maxSplit[ray][2];
  colorSplit	split;
  unsigned long colorComponent[3];
  std::list<colorSplit> lineSplit[ray];

  for (int i = 0; i < ray; i++) {
    vector = rayon;
    vector.rotate(360.0f / float(ray) * i);
    LinearDisplacement line(center + vector, vector * img->size.y);
    for (pos = line.get();
	 pos > vec2(0, 0) && pos < scany->size && !line.end();) {
      while (pos > vec2(0, 0) && pos < scany->size && !line.end() // jump edge
	     && scany->pixel[pos.to1D(img->size.x)] >= tmin)
	pos = line.get();
      split.position = pos;
      split.length = 0;
      colorComponent[0] = 0;
      colorComponent[1] = 0;
      colorComponent[2] = 0;
      for (;pos > vec2(0, 0) && pos < scany->size && !line.end()
	     && scany->pixel[pos.to1D(img->size.x)] < tmax;
	   pos = line.get()) {
	split.length++;
	split.color = img->pixel[pos.to1D(img->size.x)];
	/*	colorComponent[0] += img->pixel[pos.to1D(img->size.x)].getr();
	colorComponent[1] += img->pixel[pos.to1D(img->size.x)].getv();
	colorComponent[2] += img->pixel[pos.to1D(img->size.x)].getb();*/
	//img->pixel[pos.to1D(img->size.x)].pixel = uint16_t(i * lineSplit[i].size() * (65025 / (ray * 18)));
	//img->pixel[pos.to1D(img->size.x)].setrvb(0, 255, 0);
      }
      if (split.length > minlength) {
	/*
       colorComponent[0] /= split.length;
	colorComponent[1] /= split.length;
	colorComponent[2] /= split.length; */
	//split.color.setrvb((uint16_t)colorComponent[0], (uint16_t)colorComponent[1], (uint16_t)colorComponent[2]);
	if (lineSplit[i].size() == 1 || maxSplit[i][0].length < split.length) {
	  maxSplit[i][0] = split;
	  maxSplit[i][1] = *lineSplit[i].begin();
	}
	lineSplit[i].push_front(split);
      }
    }
  }
  int side, j, i;
  for (side = 0; side < 2; side++) {
    object.sideEdgeColor[side].clear();
    for (i = 0; i < ray; ++i) {
      //printf("color: %d\n", maxSplit[i][side].color.pixel);
      img->pixel[maxSplit[i][side].position.to1D(img->size.x)] = maxSplit[i][side].color;
      for (j = i + 1; j < ray; ++j) {
	// equivalent color, skiping first, use seconde color for both
	if (maxSplit[i][side].color.diff(maxSplit[j][side].color) < 3) { // %
	  /*maxSplit[j][side].color
	    .setrvb((uint16_t)(long(maxSplit[j][side].color.getr())
			       + maxSplit[i][side].color.getr()) / 2,
		    (uint16_t)(long(maxSplit[j][side].color.getv())
			       + maxSplit[i][side].color.getv()) / 2,
		    (uint16_t)(long(maxSplit[j][side].color.getb())
		    + maxSplit[i][side].color.getb()) / 2);*/
	  break;
	}
      }
      if (j == ray) { // no equivalence with other color found
	printf("sideEdge %d %d -> %d\n", i, side, maxSplit[i][side].color.pixel);
	object.sideEdgeColor[side].push_front(maxSplit[i][side].color);
      }
    }
  }
  return object;
}

bool	ObjectPicker::setLock(void * l) {
  objectFeature newlock = *(ObjectPicker::objectFeature *)l;

  if (newlock.sideEdgeColor[0].size() > 0 && newlock.sideEdgeColor[1].size() > 0) {
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
