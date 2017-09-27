#ifndef CONVOLUTION_HPP_
# define CONVOLUTION_HPP_

void convolution(const pixel_t *in, pixel_t *out,
		 const float *kernel, const int kn,
		 const vec2 n);

#endif /* !CONVOLUTION_HPP_ */
