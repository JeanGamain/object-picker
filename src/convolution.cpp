#include <assert.h>
#include <stdlib.h>
#include "math.hpp"
#include "pixelf.hpp"
#include "pixel16.hpp"
#include <stdio.h>

void convolution(const pixelf *in, pixelf *out,
		   const pixelf *kernel, const int ksize,
		   const vec2 n)
{
  assert(ksize % 2 == 1);
  assert(n.x > ksize && n.y > ksize);
  const int khalf = ksize / 2; 
  int c;
  float p;
  vec2 b;
 
  vec2 a;
  for (a.x = khalf; a.x < n.x - khalf; a.x++) {
    for (a.y = khalf; a.y < n.y - khalf; a.y++) {
      p = 0;
      c = 0;
      for (b.y = -khalf; b.y <= khalf; b.y++) {
	for (b.x = -khalf; b.x <= khalf; b.x++) {
	  p += in[(a.y - b.y) * n.x + a.x - b.x].get() * kernel[c].get();
	  c++;
	}
      }
      out[a.y * n.x + a.x].set(p);
    }
  }
}
