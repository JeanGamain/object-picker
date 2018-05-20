#ifndef GAUSSIAN_HPP_
# define GAUSSIAN_HPP_

#include "pixelf.hpp"
#include "vec2.hpp"

class Gaussian {
public:
  Gaussian();
  Gaussian(vec2 const & n, const float sigma);
  ~Gaussian();
  
  void	filter(const pixelf *in, pixelf *out) const;
  void	createKernel();
  int	getKernelSize() const;
  int	getKernelSize(const float a) const;

public:
  const int	ksize;
  const float	mean;
  pixelf *	kernel;
  const float	sigma;
};

#endif /* !GAUSSIAN_HPP_ */
