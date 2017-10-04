#include <stdlib.h>
#include "math.h"
#include "convolution.hpp"
#include "canny.hpp"
#include "pixelf.hpp"

/*
 * gaussianFilter:
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 * determine size of kernel (odd #)
 * 0.0 <= sigma < 0.5 : 3
 * 0.5 <= sigma < 1.0 : 5
 * 1.0 <= sigma < 1.5 : 7
 * 1.5 <= sigma < 2.0 : 9
 * 2.0 <= sigma < 2.5 : 11
 * 2.5 <= sigma < 3.0 : 13 ...
 * kernelSize = 2 * int(2*sigma) + 3;
 */
void gaussian_filter(const pixelf *in, pixelf *out,
		     const vec2 n, const float sigma)
{
  const int ksize = 2 * (int)(2 * sigma) + 3;
  const float mean = (float)floor(ksize / 2.0);
  pixelf kernel[ksize * ksize];

  size_t c = 0;
  for (int x = 0; x < ksize; x++) {
    for (int y = 0; y < ksize; y++) {
      kernel[c] = (float)(exp(-0.5 * (pow((x - mean) / sigma, 2.0) +
				       pow((y - mean) / sigma, 2.0)))
			   / (2 * M_PI * sigma * sigma));
      c++;
    }
  }

  convolution(in, out, kernel, ksize, n);
}
