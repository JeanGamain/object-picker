#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <cmath>
#include <list>
#include <omp.h>
#include "pixelf.hpp"
#include "convolution.hpp"
#include "Gaussian.hpp"
#include "math.hpp"

#include "pixel16.hpp"
extern image<pixel16> * img;

/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */

Canny::Canny(vec2 const & size,
	     unsigned int dump = 45,
	     unsigned int minlength = 6,
	     const float tmin = 50,
	     const float tmax = 60,
	     const float sigma = 1.4f)
  : size(size),
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
  boundClear = new image<pixelf>(size);
  G->clear();
  Gx->clear();
  Gy->clear();
  nms->clear();
  boundClear->clear();
  for (int x = 0; x < size.x; ++x) {
      boundClear->pixel[x] = 255.0f;
      boundClear->pixel[(size.y - 1) * size.x + x] = 255.0f;
      //img->pixel[x].setrvb(0, 255, 0);
      //img->pixel[(size.y - 1) * size.x + x].setrvb(0, 255, 0);
  }
  int sx = size.x - 1;
  for (int y = 0; y < size.y; ++y) {
      boundClear->pixel[y * size.x] = 255.0f;
      boundClear->pixel[y * size.x + sx] = 255.0f;
      //img->pixel[y * size.x].setrvb(255, 0, 0);
      //img->pixel[y * size.x + sx].setrvb(0, 0, 255);
  }
  // Reuse array
  edges = (vec2*)Gx->pixel;
}

Canny::~Canny() {
  delete(G);
  delete(Gx);
  delete(Gy);
  delete(nms);
  delete(blur);
}


std::list<Canny::edge> * Canny::edgeDetection(image<pixelf> * in)
{
  assert(in->pixel != NULL);

  image<pixelf> * newin = in;
    
  if (blur != NULL) {
    memcpy(nms->pixel, newin->pixel, size.x * size.y * sizeof(pixelf));
    //memset(nms->pixel, 1, sizeof(pixelf) * size.x * size.y);
    blur->filter(newin->pixel, nms->pixel);
    newin = nms;
  }
  
  convolution(newin->pixel, Gx->pixel, GMx, 3, size);
  convolution(newin->pixel, Gy->pixel, GMy, 3, size);

  omp_set_dynamic(0);
  omp_set_num_threads(omp_get_num_threads());
  #pragma omp for
  for (int x = 1; x < (size.x * size.y); x++) {
    G->pixel[x].set((float)(hypot(Gx->pixel[x].get(), Gy->pixel[x].get())));
    //G->pixel[c].set(ABS(after_Gx->pixel[c].get() + after_Gy->pixel[c].get()));
  }
  
  // Non-maximum suppression, straightforward implementation.
  #pragma omp for
  for (int x = 1; x < size.x - 1; x++) {
    for (int y = 1; y < size.y - 1; y++) {
      const int c = size.x * y + x;

      const float eDir = (float)(std::fmod(std::atan2(Gy->pixel[c].get(),
						      Gx->pixel[c].get()) + M_PI,
				     M_PI) / M_PI) * 8;      
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

  memcpy(G->pixel, boundClear->pixel, size.x * size.y * sizeof(pixelf));
  
  vec2 p;
  size_t c = 1;
  int nedges;
  size_t pos1d;
  vec2 newpos;
  vec2 newpossave;
  Canny::edge newedge;
  float color = 1;
  std::list<edge> * edgeList = new std::list<edge>();
  for (p.y = 1; p.y < size.y - 1; p.y++) {
    for (p.x = 1; p.x < size.x - 1; p.x++) {
      c = p.to1D(size.x);
      if (nms->pixel[c] >= tmax && G->pixel[c] < 1.0f) { // trace edge
	nedges = 1;
	newedge.pos = vec2(cordinate(c), 0);
	newedge.loop = false;
	newedge.length = 1;
	newedge.color = (unsigned int)color;
	newedge.point = new std::list<vec2>();
	G->pixel[c].set(color); // colo
	edges[0] = p;
	do {
	  nedges--;
	  newpossave = edges[nedges];
	  for (int k = 0; k < 8; k++) {
	    newpos = newpossave + dir[k];
	    pos1d = (newpos).to1D(size.x); // what what
	    if (nms->pixel[pos1d] >= tmin
		&& G->pixel[pos1d] < 1.0f) {
	      /*if (newpos < vec2(1, 1) || newpos >= vec2(size.x - 1, size.y - 1)) {
		printf("newpos: %d %d c: %f t: %f\n", newpos.x, newpos.y, G->pixel[pos1d].pixel, G->pixel[newpos.to1D(size.x)].pixel);
		continue;
		}*/
	      G->pixel[pos1d].set(color);
	      edges[nedges] = newpos;
	      nedges++;
	      newedge.length++;
	      if (dump == 0 || newedge.length % dump == 0) {
		newedge.point->push_front(newpos);
	      }
	    }
	    if (G->pixel[pos1d] == color) {
	      newedge.loop = true;
	    }
	  }
	} while (nedges > 0);
	if (newedge.length > minlength) {
	  edgeList->push_front(newedge);
	  color = CIRCULAR_ADD(color, 254, 1) + 1;
	}
      }
    }
  }
  return edgeList;
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
