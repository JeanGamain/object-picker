#include <assert.h>
#include <stdlib.h>
#include "math.hpp"
#include "pixel.hpp"
#include "pixelf.hpp"
#include <stdio.h>

void convolution(const pixel *in, pixel *out,
		   const pixel *kernel, const int ksize,
		   const vec2 n)
{
  assert(ksize % 2 == 1);
  assert(n.x > ksize && n.y > ksize);
  const int khalf = ksize / 2;
  
  vec2 a;
  for (a.x = khalf; a.x < n.x - khalf; a.x++) {
    for (a.y = khalf; a.y < n.y - khalf; a.y++) {
      float pixel = 0;
      size_t c = 0;
      vec2 b;
   
      for (b.y = -khalf; b.y <= khalf; b.y++) {
	for (b.x = -khalf; b.x <= khalf; b.x++) {
	  pixel += in[(a.y - b.y) * n.x + a.x - b.x].get()
	    * kernel[c].get();
	  c++;
	}
      }
      out[a.y * n.x + a.x].set(pixel);
    }
  }
}
