#include <stdlib.h>
#include "math.hpp"
#include "Convolution.hpp"
#include "Canny.hpp"
#include "pixelf.hpp"

// http://www.songho.ca/dsp/cannyedge/cannyedge.html

Gaussian::Gaussian()
  :
  ksize(7),
  mean((float)floor(ksize / 2.0)),
  sigma(1.47),
  kernel(NULL)  
{
  /*
    kernel = {
      0.001144, 0.003638, 0.007283, 0.009179, 0.007283, 0.003638, 0.001144,
      0.003638, 0.011568, 0.023160, 0.029190, 0.023160, 0.011568, 0.003638,
      0.007283, 0.023160, 0.046367, 0.058438, 0.046367, 0.023160, 0.007283,
      0.009179, 0.029190, 0.058438, 0.073652, 0.058438, 0.029190, 0.009179,
      0.007283, 0.023160, 0.046367, 0.058438, 0.046367, 0.023160, 0.007283,
      0.003638, 0.011568, 0.023160, 0.029190, 0.023160, 0.011568, 0.003638,
      0.001144, 0.003638, 0.007283, 0.009179, 0.007283, 0.003638, 0.001144
    };
  */
}

Gaussian::Gaussian(const float sigma)
  : ksize(getKernelSize(sigma)),
    mean((float)floor(ksize / 2.0)),
    sigma(sigma),
    kernel(new pixelf[ksize * ksize])
{
  createKernel();
}

Gaussian::~Gaussian() {
  delete(kernel);
}

void Gaussian::createKernel()
{
    size_t c = 0;
  for (int x = 0; x < ksize; x++) {
    for (int y = 0; y < ksize; y++) {
      kernel[c] = (float)(exp(-0.5 * (pow((x - mean) / sigma, 2.0) +
				      pow((y - mean) / sigma, 2.0)))
			  / (2 * M_PI * sigma * sigma));
      
      //printf("%f, ", kernel[c].pixel);
      c++;
    }
    //printf("\n");
  }
}

int Gaussian::getKernelSize(const float a) const
{
  return 2 * (int)(2 * a) + 3;
}

int Gaussian::getKernelSize() const
{
  return 2 * (int)(2 * sigma) + 3;
}

void Gaussian::filter(vec2 const & size, const pixelf *in, pixelf *out) const
{
  convolution(in, out, kernel, ksize, size);
}
