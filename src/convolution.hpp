#ifndef CONVOLUTION_HPP_
# define CONVOLUTION_HPP_

#include "vec2.hpp"
#include "canny.hpp"
#include "pixelf.hpp"

void convolution(const pixelf *in, pixelf *out,
		 const pixelf *kernel, const int kn,
		 const vec2 n);

#endif /* !CONVOLUTION_HPP_ */
