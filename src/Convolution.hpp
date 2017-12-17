#ifndef CONVOLUTION_HPP_
# define CONVOLUTION_HPP_

#include "vec2.hpp"
#include "Canny.hpp"
#include "pixelf.hpp"

void convolution(const pixelf *in, pixelf *out,
		 const pixelf *kernel, const int kn,
		 vec2 const & n);

#endif /* !CONVOLUTION_HPP_ */
