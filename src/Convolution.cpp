#include <assert.h>
#include <stdlib.h>
#include "vec2.hpp"
#include "pixelf.hpp"
#include "pixel.hpp"
#include <stdio.h>

void convolution(const pixelf *in, pixelf *out,
		 const pixelf *kernel, const int ksize,
		 vec2 const & n)
{
  assert(ksize % 2 == 1);
  assert(n.x > ksize && n.y > ksize);
  const int khalf = ksize / 2;
  int c;
  float p;
  vec2 b;

    #pragma omp for
  for (int x = khalf; x < n.x - khalf; x++) {
    for (int y = khalf; y < n.y - khalf; y++) {
      p = 0;
      c = 0;
      for (b.y = -khalf; b.y <= khalf; b.y++) {
	for (b.x = -khalf; b.x <= khalf; b.x++) {
	  p += in[(y - b.y) * n.x + x - b.x].get() * kernel[c].get();
	  c++;
	}
      }
      if (p < 0) {
	//printf("no %f\n", p);
      }
      out[y * n.x + x].set(p);
    }
  }
}
