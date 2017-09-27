#ifndef CANNY_HPP_
# define CANNY_HPP_

pixel_t *canny_edge_detection(const pixel_t *in,
			      const bitmap_info_header_t *bmp_ih,
			      const int tmin, const int tmax,
			      const float sigma);

#endif /* !CANNY_HPP_ */
