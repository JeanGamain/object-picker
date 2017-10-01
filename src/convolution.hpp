#ifndef CONVOLUTION_HPP_
# define CONVOLUTION_HPP_

#include "vec2.hpp"
#include "canny.hpp"

void convolution(const pixel *in, pixel *out,
		 const pixel *kernel, const int kn,
		 const vec2 n);

#endif /* !CONVOLUTION_HPP_ */
