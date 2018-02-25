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
  //assert(n.x % ksize == 0 && n.y % ksize == 0);
  assert(in != out);
  int c;
  float p;
  vec2 pos;
  vec2 kpos;
  vec2 outBand;
  
  for (pos.x = 0; pos.x < (n.x - ksize); pos.x++) {
    // main (n.x % ksize == 0 && n.y % ksize == 0)
    for (pos.y = 0; pos.y < (n.y - ksize); pos.y++) {
      p = 0;
      c = 0;
      for (kpos.x = pos.x; kpos.x < (pos.x + ksize); kpos.x++) {
	for (kpos.y = pos.y; kpos.y < (pos.y + ksize); kpos.y++) {
	  p += kernel[c++].get() * in[kpos.to1D(n.x)].get();
	}
      }
      out[pos.to1D(n.x)].set(p);
    }
    // bottom
    for (outBand = vec2(0, ksize); pos.y < n.y; pos.y++) {
      p = 0;
      for (kpos.x = 0; kpos.x < ksize; kpos.x++) {
	for (kpos.y = 0; kpos.y < ksize; kpos.y++) {
	  p += kernel[kpos.to1D(ksize)].get() * in[(pos + kpos - outBand).to1D(n.x)].get();
	}
      }
      out[pos.to1D(n.x)].set(p);
      outBand.y--;
    }
  }
  // left line
  outBand = vec2(ksize, 0);
  for (; pos.x < n.x; pos.x++) {
    for (pos.y = 0; pos.y < (n.y - ksize); pos.y++) {
      p = 0;
      for (kpos.x = 0; kpos.x < ksize; kpos.x++) {
	for (kpos.y = 0; kpos.y < ksize; kpos.y++) {
	  p += kernel[kpos.to1D(ksize)].get() * in[(pos + kpos - outBand).to1D(n.x)].get();
	}
      }
      out[pos.to1D(n.x)].set(p);
    }
    outBand.x--;
  }
  // corner
  outBand.x = ksize;
  for (pos.x = n.x - ksize; pos.x < n.x; pos.x++) {
    outBand.y = ksize;
    for (pos.y = n.y - ksize; pos.y < n.y; pos.y++) {
      p = 0;
      for (kpos.x = 0; kpos.x < ksize; kpos.x++) {
	for (kpos.y = 0; kpos.y < ksize; kpos.y++) {
	  p += kernel[kpos.to1D(ksize)].get() * in[(pos + kpos - outBand).to1D(n.x)].get();
	}
      }
      out[pos.to1D(n.x)].set(p);
      outBand.y--;
    }
    outBand.x--;
  }
}
