#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "pixelf.hpp"
#include "convolution.hpp"
#include "Gaussian.hpp"
#include "math.hpp"

/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */

Canny::Canny(vec2 const & size, const pixelf color, const float tmin,
	     const float tmax, const float sigma, const float rsize)
  : truesize(size),
    size(vec2(cordinate(size.x * ((rsize > 0.0f) ? rsize : 1)), cordinate(size.y * ((rsize > 0.0f) ? rsize : 1)))),
    color(color),
    tmin(tmin),
    tmax(tmax),
    sigma(sigma),
    resize(rsize),
    blur((sigma > 0) ? new Gaussian(size, sigma) : NULL)
{
  G = new image<pixelf>(size);
  Gx = new image<pixelf>(size);
  Gy = new image<pixelf>(size);
  nms = new image<pixelf>(size);
  G->clear();
  Gx->clear();
  Gy->clear();
  nms->clear();
}

Canny::~Canny() {
  delete(G);
  delete(Gx);
  delete(Gy);
  delete(nms);
  delete(blur);
}


void Canny::edgeDetection(image<pixelf> * in,
			  image<pixelf> * out)
{
  assert(in->pixel != NULL && out->pixel != NULL);
  //assert(truesize == in->size && truesize == out->size);

  image<pixelf> * newin = in;
    
  if (resize > 0) {
    in->resize(newin, resize, resize);
    newin = G;
  }
  
  if (blur != NULL) {
    memcpy(nms->pixel, newin->pixel, size.x * size.y * sizeof(pixelf));
    //memset(nms->pixel, 1, sizeof(pixelf) * size.x * size.y);
    blur->filter(newin->pixel, nms->pixel);
    newin = nms;
  }
  
  /*
  for (int x = 0; x < newin->size.x; x++) {
    for (int y = 0; y < newin->size.y; y++) {
      out->pixel[y * out->size.x + x] = newin->pixel[y * newin->size.x + x];
    }
  }
  return ;
  */
  convolution(newin->pixel, Gx->pixel, GMx, 3, size);
  
  convolution(newin->pixel, Gy->pixel, GMy, 3, size);
  
  for (int x = 1; x < size.x - 1; x++) {
    for (int y = 1; y < size.y - 1; y++) {
      const int c = y * size.x + x;
      G->pixel[c].set((float)(hypot(Gx->pixel[c].get(), Gy->pixel[c].get())));
      //G->pixel[c].set(ABS(after_Gx->pixel[c].get() + after_Gy->pixel[c].get()));
    }
  }
  
  // Non-maximum suppression, straightforward implementation.
  vec2 p;
  int nord;
  int sud;
  for (p.x = 1; p.x < size.x - 1; p.x++) {
    for (p.y = 1; p.y < size.y - 1; p.y++) {
      const int c = size.x * p.y + p.x;

      const float eDir = (float)(fmod(atan2(Gy->pixel[c].get(),
					   Gx->pixel[c].get()) + M_PI,
				     M_PI) / M_PI) * 8;      
      nord = c - size.x;
      sud = c + size.x;
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

  // Reuse array
  vec2 *edges = (vec2*) Gx->pixel; // realloc --
  memset(out->pixel, 0, sizeof(pixelf) * size.x * size.y);
  int sx = size.x - 1;
  for (int x = 1; x < size.x; ++x) {
      out->pixel[x] = 255.0f;
      out->pixel[(size.y - 1) * size.x + x] = 255.0f;
  }
  for (int y = 1; y < size.y - 1; ++y) {
      out->pixel[y * size.x] = 255.0f;
      out->pixel[y * size.x + sx] = 255.0f;
  }

  // Tracing edges with hysteresis . Non-recursive implementation.
  size_t c = 1;
  int pos1d;
  vec2 newpos;
  for (p.y = 1; p.y < size.y - 1; p.y++) {
    for (p.x = 1; p.x < size.x - 1; p.x++) {
      if (nms->pixel[c] >= tmax && out->pixel[c] == 0.0) { // trace edges
	out->pixel[c].set(color);
	int nedges = 1;
	edges[0] = p;
	do {
	  nedges--;
	  for (int k = 0; k < 8; k++) {
	    pos1d = (newpos).to1D(size.x);
	    if (nms->pixel[pos1d] >= tmin && out->pixel[pos1d] == 0.0) {
	      out->pixel[pos1d].set(color);
	      edges[nedges] = newpos;
	      nedges++;
	    }
	  }
	} while (nedges > 0);
      }
      c++;
    }
  }
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

void Canny::setResize(float r) {
  if (r <= 0) {
    size = truesize;
  } else {
    size = vec2(cordinate(r * truesize.x), cordinate(r * truesize.y));
  }
  G->setSize(size);
  Gx->setSize(size);
  Gy->setSize(size);
  nms->setSize(size);
  resize = r;
}

void Canny::setColor(pixelf c) {
  color = c;
}


float	Canny::getMax() {
  return tmax;
}

float	Canny::getMin() {
  return tmin;
}

float	Canny::getBlur() {
  return sigma;
}

float	Canny::getResize() {
  return resize;
}

pixelf	Canny::getColor() {
  return color;
}
