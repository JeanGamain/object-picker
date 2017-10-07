#ifndef CANNY_HPP_
# define CANNY_HPP_

#include <stdint.h>
#include "pixelf.hpp"
#include "image.hpp"
#include "math.hpp"
#include "Gaussian.hpp"


class Canny {
public:
  Canny(vec2 const & size, const pixelf color, const float tmin, const float tmax, const float sigma);
  ~Canny();
  
  void edgeDetection(const image<pixelf> * in, const image<pixelf> * out); // use generic image type

  void setMax(float max);
  void setMin(float min);
  void setBlur(float s);
  void setColor(pixelf color);

  float getMax();
  float getMin();
  float getBlur(); // blur interface?
  pixelf getColor();
  
private:
  vec2		size;
  pixelf	color;
  float		tmin;
  float		tmax;
  float		sigma;
  Gaussian *	blur;
  
  image<pixelf>	 *G; // & const
  image<pixelf>  *Gx;
  image<pixelf>	 *Gy;
  image<pixelf>  *nms;
  
  const pixelf GMx[9] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
  };
  
  
  const pixelf GMy[9] = {
    1, 2, 1,
    0, 0, 0,
    -1,-2,-1
  };
};

#endif /* !CANNY_HPP_ */
