#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "convolution.hpp"
#include "gaussian.hpp"

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t nothing;
} rgb_t;

/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */
pixel_t *canny_edge_detection(const pixel_t *in,
			      const bitmap_info_header_t *bmp_ih,
			      const int tmin, const int tmax,
			      const float sigma)
{
  const vec2 n = image.size;

  pixel_t *G = calloc(n.x * n.y * sizeof(pixel_t), 1); // use malloc (new ?)
  pixel_t *after_Gx = calloc(n.x * n.y * sizeof(pixel_t), 1);
  pixel_t *after_Gy = calloc(n.x * n.y * sizeof(pixel_t), 1);
  pixel_t *nms = calloc(n.x * n.y * sizeof(pixel_t), 1);
  pixel_t *out = malloc(bmp_ih->bmp_bytesz * sizeof(pixel_t));

  if (G == NULL || after_Gx == NULL || after_Gy == NULL ||
      nms == NULL || out == NULL) {
    fprintf(stderr, "canny_edge_detection:"
	    " Failed memory allocation(s).\n");
    exit(1);
  }

  gaussian_filter(in, out, n, sigma);

  const float Gx[] = {-1, 0, 1,
		      -2, 0, 2,
		      -1, 0, 1};

  convolution(out, after_Gx, Gx, 3, n);

  const float Gy[] = { 1, 2, 1,
		       0, 0, 0,
		       -1,-2,-1};

  convolution(out, after_Gy, Gy, 3, n);

  for (int x = 1; x < n.x - 1; x++)
    for (int y = 1; y < n.y - 1; y++) {
      const int c = y + n.x * x;
      // G[c] = abs(after_Gx[c]) + abs(after_Gy[c]);
      G[c] = (pixel_t)hypot(after_Gx[c], after_Gy[c]);
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

      const float dir = (float)(fmod(atan2(after_Gy[c],
					   after_Gx[c]) + M_PI,
				     M_PI) / M_PI) * 8;

      if (((dir <= 1 || dir > 7) && G[c] > G[ee] &&
	   G[c] > G[ww]) || // 0 deg
	  ((dir > 1 && dir <= 3) && G[c] > G[nw] &&
	   G[c] > G[se]) || // 45 deg
	  ((dir > 3 && dir <= 5) && G[c] > G[nn] &&
	   G[c] > G[ss]) || // 90 deg
	  ((dir > 5 && dir <= 7) && G[c] > G[ne] &&
	   G[c] > G[sw]))   // 135 deg
	nms[c] = G[c];
      else
	nms[c] = 0;
    }
  }
  
  // Reuse array
  // used as a stack. nx*ny/2 elements should be enough.
  int *edges = (int*) after_Gy; // realloc --
  memset(out, 0, sizeof(pixel_t) * n.x * n.y);
  memset(edges, 0, sizeof(pixel_t) * n.x * n.y);

  // Tracing edges with hysteresis . Non-recursive implementation.
  size_t c = 1;
  for (int y = 1; y < n.y - 1; y++)
    for (int x = 1; x < n.x - 1; x++) {
      if (nms[c] >= tmax && out[c] == 0) { // trace edges
	out[c] = MAX_BRIGHTNESS; // IFX --------
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
	    if (nms[nbs[k]] >= tmin && out[nbs[k]] == 0) {
	      out[nbs[k]] = MAX_BRIGHTNESS;
	      edges[nedges] = nbs[k];
	      nedges++;
	    }
	} while (nedges > 0);
      }
      c++;
    }

  free(after_Gx);
  free(after_Gy);
  free(G);
  free(nms);

  return out;
}
