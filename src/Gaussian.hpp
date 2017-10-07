#ifndef GAUSSIAN_HPP_
# define GAUSSIAN_HPP_

#include "pixelf.hpp"
#include "vec2.hpp"

class Gaussian {
public:
  Gaussian(const vec2 n, const float sigma);
  ~Gaussian();
  
  void filter(const pixelf *in, pixelf *out);

public:
  const vec2	size;
  const int	ksize;
  const float	mean;
  pixelf *	kernel;
  const float	sigma;
};

#endif /* !GAUSSIAN_HPP_ */