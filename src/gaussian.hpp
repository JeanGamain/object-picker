#ifndef GAUSSIAN_HPP_
# define GAUSSIAN_HPP_

#include "pixelf.hpp"

void gaussian_filter(const pixelf *in, pixelf *out,
		     const vec2 n, const float sigma);

#endif /* !GAUSSIAN_HPP_ */
