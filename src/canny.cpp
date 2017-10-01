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
#include "gaussian.hpp"

/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */
void canny_edge_detection(const image<pixel16> * in,
			  const image<pixel16> * out,
			  const int tmin, const int tmax,
			  const float sigma)
{
  assert(in->pixel != NULL && out->pixel != NULL);
  const vec2 n = in->size;

  image<pixelf> *G = new image<pixelf>(n);
  image<pixelf>  *after_Gx = new image<pixelf>(n);
  image<pixelf>  *after_Gy = new image<pixelf>(n);
  image<pixelf>  *nms = new image<pixelf>(n);
 

  gaussian_filter(in->pixel, out->pixel, n, sigma);

  const pixelf Gx[] = {-1, 0, 1,
		       -2, 0, 2,
		       -1, 0, 1};
  
  convolution(out->pixel, after_Gx->pixel, Gx, 3, n);

  const pixelf Gy[] = { 1, 2, 1,
			0, 0, 0,
			-1,-2,-1};
  
  convolution(out->pixel, after_Gy->pixel, Gy, 3, n);

  for (int x = 1; x < n.x - 1; x++)
    for (int y = 1; y < n.y - 1; y++) {
      const int c = y + n.x * x;
      // G[c] = abs(after_Gx[c]) + abs(after_Gy[c]);
      G->pixel[c].set((uint16_t)(hypot(after_Gx->pixel[c].get(), after_Gy->pixel[c].get())));
    }

  // Non-maximum suppression, straightforward implementation.
  for (int x = 1; x < n.x - 1; x++) {
    for (int y = 1; y < n.y - 1; y++) {
      const int c = x + n.x * y;
      const int nn = c - n.x;
      const int ss = c + n.x;
      const int ww = c + 1;
      const int ee = c - 1;
      const int nw = nn + 1;
      const int ne = nn - 1;
      const int sw = ss + 1;
      const int se = ss - 1;

      const float dir = (float)(fmod(atan2(after_Gy->pixel[c].get(),
					   after_Gx->pixel[c].get()) + M_PI,
				     M_PI) / M_PI) * 8;

      if (((dir <= 1 || dir > 7) && G->pixel[c] > G->pixel[ee] &&
	   G->pixel[c] > G->pixel[ww]) || // 0 deg
	  ((dir > 1 && dir <= 3) && G->pixel[c] > G->pixel[nw] &&
	   G->pixel[c] > G->pixel[se]) || // 45 deg
	  ((dir > 3 && dir <= 5) && G->pixel[c] > G->pixel[nn] &&
	   G->pixel[c] > G->pixel[ss]) || // 90 deg
	  ((dir > 5 && dir <= 7) && G->pixel[c] > G->pixel[ne] &&
	   G->pixel[c] > G->pixel[sw]))   // 135 deg
	nms->pixel[c] = G->pixel[c];
      else
	nms->pixel[c] = 0;
    }
  }
  
  // Reuse array
  // used as a stack. nx*ny/2 elements should be enough.
  int *edges = (int*) after_Gy->pixel; // realloc --
  memset(out->pixel, 0, sizeof(pixelf) * n.x * n.y);
  memset(edges, 0, sizeof(int) * n.x * n.y);

  // Tracing edges with hysteresis . Non-recursive implementation.
  size_t c = 1;
  for (int y = 1; y < n.y - 1; y++)
    for (int x = 1; x < n.x - 1; x++) {
      if (nms->pixel[c] >= tmax && out->pixel[c] == 0) { // trace edges
	out->pixel[c] = 0xffff;
	int nedges = 1;
	edges[0] = c;

	do {
	  nedges--;
	  const int t = edges[nedges];

	  int nbs[8]; // neighbours
	  nbs[0] = t - n.x;     // nn
	  nbs[1] = t + n.x;     // ss
	  nbs[2] = t + 1;      // ww
	  nbs[3] = t - 1;      // ee
	  nbs[4] = nbs[0] + 1; // nw
	  nbs[5] = nbs[0] - 1; // ne
	  nbs[6] = nbs[1] + 1; // sw
	  nbs[7] = nbs[1] - 1; // se

	  for (int k = 0; k < 8; k++)
	    if (nms->pixel[nbs[k]] >= tmin && out->pixel[nbs[k]] == 0) { // make == >= operator
	      out->pixel[nbs[k]].set(0xff);
	      edges[nedges] = nbs[k];
	      nedges++;
	    }
	} while (nedges > 0);
      }
      c++;
    }
  delete(G);
  delete(after_Gx);
  delete(after_Gy);
  delete(nms);
}