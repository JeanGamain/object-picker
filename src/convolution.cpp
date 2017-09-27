#include <assert.h>
#include <stdlib.h>
#include "math.hpp"

/*
// if normalize is true, map pixels to range 0..MAX_BRIGHTNESS

  if (normalize)
    for (int m = khalf; m < nx - khalf; m++)
      for (int n = khalf; n < ny - khalf; n++) {
	float pixel = 0.0;
	size_t c = 0;
	for (int j = -khalf; j <= khalf; j++)
	  for (int i = -khalf; i <= khalf; i++) {
	    pixel += in[(n - j) * nx + m - i] * kernel[c];
	    c++;
	  }
	if (pixel < min)
	  min = pixel;
	if (pixel > max)
	  max = pixel;
      }


      #define MAX_BRIGHTNESS 255
 */

void convolution(const pixel_t *in, pixel_t *out,
		 const float *kernel, const int ksize,
		 const vec2 n)
{
  assert(ksize % 2 == 1);
  assert(n.x > ksize && n.y > ksize);
  const int khalf = ksize / 2;

  vec2 a;
  for (a.x = khalf; a.x < n.x - khalf; a.x++)
    for (a.y = khalf; a.y < n.y - khalf; a.y++) {
      float pixel = 0.0;
      size_t c = 0;
      vec2 b;
      for (b.y = -khalf; j <= khalf; b.y++)
	for (b.x = -khalf; i <= khalf; b.x++) {
	  pixel += in[(a.y - b.y) * n.x + a.x - b.x] * kernel[c];
	  c++;
	}

      /*
      if (normalize)
	pixel = MAX_BRIGHTNESS * (pixel - min) / (max - min);
      */
      out[a.y * n.x + a.x] = (pixel_t)pixel;
    }
}
