#ifndef GAUSSIAN_HPP_
# define GAUSSIAN_HPP_

void gaussian_filter(const pixel_t *in, pixel_t *out,
		     const vec2 n, const float sigma);

#endif /* !GAUSSIAN_HPP_ */
