#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <cmath>
#include <list>
#include "pixelf.hpp"
#include "Convolution.hpp"
#include "Gaussian.hpp"
#include "math.hpp"

#include "pixel.hpp"
#include "parm.hpp"
extern image<pixel> * img;
extern varSet vaParm[24];
extern int maxParm;
extern unsigned int renderMode;

Canny::Canny(vec2 const & size,
	     unsigned char * state,
	     unsigned int dump = 45,
	     unsigned int minlength = 6,
	     const float tmin = 50,
	     const float tmax = 60,
	     const float sigma = 1.4f)
  : size(size),
    edgeGroupId(Canny::minEdgeGroupId),
    detectionState(state),
    dump(dump),
    minlength(minlength),
    tmin(tmin),
    tmax(tmax),
    sigma(sigma),
    blur((sigma > 0) ? new Gaussian(size, sigma) : NULL)
{
  G = new image<pixelf>(size);
  Gx = new image<pixelf>(size);
  Gy = new image<pixelf>(size);
  nms = new image<pixelf>(size);
  boundClearScan = new unsigned char[size.x * size.y];
 
  G->clear();
  Gx->clear();
  Gy->clear();
  nms->clear();
  memset(boundClearScan, 0, sizeof(char) * size.x * size.y);
  for (int x = 0; x < size.x; ++x) {
      boundClearScan[x] = 255;
      boundClearScan[(size.y - 1) * size.x + x] = 255;
  }
  int sx = size.x - 1;
  for (int y = 0; y < size.y; ++y) {
      boundClearScan[y * size.x] = 255;
      boundClearScan[y * size.x + sx] = 255;
  }
  // Reuse array
  edges = (int*)Gx->pixel;
  edgeList = new std::list<edge>();

  static float maxMin = 1, maxMax = 200, maxStep = 1;
  vaParm[maxParm++] = (varSet){ &maxMin, &maxMax, &maxStep, &this->tmax, "canny max", FLOAT };
  static float minMin = 1, minMax = 200, minStep = 1;
  vaParm[maxParm++] = (varSet){ &minMin, &minMax, &minStep, &this->tmin, "canny min", FLOAT };
}

Canny::~Canny() {
  delete(G);
  delete(Gx);
  delete(Gy);
  delete(nms);
  delete(blur);
  delete(boundClearScan);
  delete(edgeList);
}


image<pixelf> * Canny::scan(image<pixelf> * in)
{
  assert(in->pixel != NULL);

  image<pixelf> * newin = in;

  // clear output
  if (blur != NULL) {
    memcpy(nms->pixel, newin->pixel, size.x * size.y * sizeof(pixelf));
    blur->filter(newin->pixel, nms->pixel);
    newin = nms;
  }
  
  convolution(newin->pixel, Gx->pixel, GMx, 3, size);
  convolution(newin->pixel, Gy->pixel, GMy, 3, size);

  #pragma omp parallel for
  for (int x = 1; x < (size.x * size.y); x++) {
    G->pixel[x].set((float)(hypot(Gx->pixel[x].get(), Gy->pixel[x].get())));
    //G->pixel[x].set(ABS(Gx->pixel[x].get() + Gy->pixel[x].get()));
  }

  #pragma omp parallel for
  for (int x = 1; x < size.x - 1; x++) {
    for (int y = 1; y < size.y - 1; y++) {
      const int c = size.x * y + x;

      const float eDir = (npixel)(fmodf(atan2f(Gy->pixel[c].get(),
						      Gx->pixel[c].get()) + M_PI,
				     M_PI) / M_PI * 8);
      int nord = c - size.x;
      int sud = c + size.x;
      if (	  
	  ((eDir > 3 && eDir <= 5)			// 90°
	   && G->pixel[c] > G->pixel[nord]		// N
	   && G->pixel[c] > G->pixel[sud]) ||		// S
	  
	  ((eDir <= 1 || eDir > 7)			// 0°
	   && G->pixel[c] > G->pixel[c - 1]		// W
	   && G->pixel[c] > G->pixel[c + 1]) ||		// E

	  ((eDir > 1 && eDir <= 3)			// 45°
	   && G->pixel[c] > G->pixel[nord + 1]		// NE
	   && G->pixel[c] > G->pixel[sud - 1]) ||	// SW

	  ((eDir > 5 && eDir <= 7)			// 135°
	   && G->pixel[c] > G->pixel[nord - 1]		// NW
	   && G->pixel[c] > G->pixel[sud + 1])		// SE
		  )
	nms->pixel[c] = G->pixel[c];
      else
	nms->pixel[c] = 0;
    }
  }
  return nms;
}

bool		Canny::getEdge(edge & newedge,
			       cordinate position,
			       unsigned int dump,
			       XrayFeatures::xrayFeatures const & features,
			       image<pixel> const & image,
			       float maxPixelDiff,
			       bool start) {
  const vec2	halfImgSize = image.size / 2;
  unsigned int	nedges;
  cordinate	kdir[9];
  cordinate	pos1d;

  // nms->pixel[position] < tmax ||
  if (detectionState[position] > 0)
    return false;
  nedges = 1;
  newedge.position = position;
  newedge.length = 1;
  newedge.color = 0; // use for pixel moy of the edge 
  newedge.point = new std::list<edgePoint>();
  detectionState[position] = edgeGroupId;
  image.pixel[position].set((uint32_t)((uint32_t)16581375 / 100 * edgeGroupId + 10) ^ 1);
  edges[0] = position;
  do {
    kdir[8] = edges[--nedges];
    kdir[0] = kdir[8] + size.x;
    kdir[1] = kdir[8] - size.x;
    kdir[2] = kdir[8] + 1;
    kdir[3] = kdir[8] - 1;
    kdir[4] = kdir[0] + 1;
    kdir[5] = kdir[0] - 1;
    kdir[6] = kdir[1] + 1;
    kdir[7] = kdir[1] - 1;
    for (int k = 0; k < 8; k++) {
      pos1d = kdir[k];
      if (nms->pixel[pos1d] >= tmin
	  && detectionState[pos1d] < 1) {
	//if (pos1d < (size.x * 1 + 1) || pos1d >= ((size.x - 1) + (size.y - 1) * size.x)) {
	// printf("NOO\n");
	// continue;
	// }
	detectionState[pos1d] = edgeGroupId;
	edges[nedges] = pos1d;
	nedges++;
	newedge.length++;
	if (dump == 0 || newedge.length % dump == 0) {
	  if (renderMode == 3) {
	    image.pixel[pos1d].setrvb(0, 0, 255);
	    continue;
	  }
	  int dk;
	  vec2 pos(pos1d % image.size.x, pos1d / image.size.x);
	  
	  if (k == 1 || k == 6) {
	    dk = (pos.x < halfImgSize.x) ? 3 : 4;
	  } else if (k == 3 || k == 4) {
	    dk = (pos.y < halfImgSize.y) ? 1 : 6;
          } else {
	    dk =
	      (pos.x < halfImgSize.x) ?
	      (pos.y < halfImgSize.y ? 0 : 5) :
	      (pos.y < halfImgSize.y ? 2 : 7);
	  }
	  std::list<pixel>::const_iterator i;
	  int innerColor = -1;
	  pixel edgeColor[2] = {
	    image.pixel[(dirNormal[start][dk] * 2).to1D(image.size.x) + pos1d],
	    image.pixel[(dirNormal[!start][dk] * 2).to1D(image.size.x) + pos1d]
	  };
	  image.pixel[(dirNormal[start][dk] * 3).to1D(image.size.x) + pos1d].setrvb(0, 255, 0);
	  image.pixel[(dirNormal[!start][dk] * 3).to1D(image.size.x) + pos1d].setrvb(0, 0, 255);
	  for (i = features.backgroundColor.begin();
	       i != features.backgroundColor.end() && innerColor < 0;
	       i++) {
	    if ((*i).diff(edgeColor[0]) <= maxPixelDiff)
	      innerColor = 1;
	    else if ((*i).diff(edgeColor[1]) <= maxPixelDiff)
	      innerColor = 0;
	  }
	  image.pixel[pos1d].setrvb(255, 0, 0);
	  //	  image.pixel[pos1d].set((uint32_t)((uint32_t)16581375 / 100 * edgeGroupId + 10) ^ 1);
	  if (innerColor > -1) {
	    for (i = features.objectColor.begin();
		 i != features.objectColor.end() && (*i).diff(edgeColor[innerColor]) > maxPixelDiff;
		 i++);
	    if (i != features.objectColor.end()) {
	      newedge.point->push_front({ pos1d, (char)dk });
	    } else {
	      continue;
	    }
	  } else {
	    continue;
	  }
	}
      }
    }
  } while (nedges > 0);
  edgeGroupId++;
  return true;
}

void				Canny::clearState() {
  memcpy(detectionState, boundClearScan, size.x * size.y * sizeof(char));
  edgeGroupId = Canny::minEdgeGroupId;
}

void Canny::setMax(float max) {
  tmax = max;
}

void Canny::setMin(float min) {
  tmin = min;
}

void Canny::setBlur(float s) {
  sigma = s;
  if (blur != NULL)
    delete(blur);
  blur = NULL;
  if (s > 0) {
    blur = new Gaussian(size, sigma);
  }
}

void Canny::setDump(unsigned int d) {
  dump = d;
}

void Canny::setMinLength(unsigned int l) {
  minlength = l;
}

float	Canny::getMax() const {
  return tmax;
}

float	Canny::getMin() const {
  return tmin;
}

float	Canny::getBlur() const {
  return sigma;
}

unsigned int Canny::getDump() const {
  return dump;
}

unsigned int Canny::getMinLength() const {
  return minlength;
}
