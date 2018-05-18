#include <stdlib.h>
#include "math.hpp"
#include "Convolution.hpp"
#include "Canny.hpp"
#include "pixelf.hpp"

// http://www.songho.ca/dsp/cannyedge/cannyedge.html

Gaussian::Gaussian(vec2 const & n, const float sigma)
  : size(n),
    ksize(getKernelSize(sigma)),
    mean((float)floor(ksize / 2.0)),
    kernel(new pixelf[ksize * ksize]),
    sigma(sigma)
{
  size_t c = 0;
  for (int x = 0; x < ksize; x++) {
    for (int y = 0; y < ksize; y++) {
      kernel[c] = (float)(exp(-0.5 * (pow((x - mean) / sigma, 2.0) +
				      pow((y - mean) / sigma, 2.0)))
			  / (2 * M_PI * sigma * sigma));
      
      c++;
    }
  }
}

Gaussian::~Gaussian() {
  delete(kernel);
}

int Gaussian::getKernelSize(const float a) const
{
  return 2 * (int)(2 * a) + 3;
}

int Gaussian::getKernelSize() const
{
  return 2 * (int)(2 * sigma) + 3;
}

void Gaussian::filter(const pixelf *in, pixelf *out) const
{
  convolution(in, out, kernel, ksize, size);
}
