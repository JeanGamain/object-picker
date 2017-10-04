#ifndef CANNY_HPP_
# define CANNY_HPP_

#include <stdint.h>
#include "pixel16.hpp"
#include "image.hpp"
#include "math.hpp"

void canny_edge_detection(const image<pixel16> * in,
			  const image<pixel16> * out,
			  const float tmin, const float tmax,
			  const float sigma); // use generic image type

#endif /* !CANNY_HPP_ */
