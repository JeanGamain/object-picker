#ifndef CANNY_HPP_
# define CANNY_HPP_

#include <stdint.h>
#include "pixelf.hpp"
#include "image.hpp"
#include "math.hpp"
#include "Gaussian.hpp"


class Canny {
public:
  Canny(vec2 const & size, const pixelf color, const float tmin, const float tmax, const float sigma, const float rsize);
  ~Canny();
  
  void edgeDetection(image<pixelf> * in, image<pixelf> * out); // use generic image type

  void setMax(float max);
  void setMin(float min);
  void setBlur(float s);
  void setResize(float r);
  void setColor(pixelf color);
  
  
  float getMax();
  float getMin();
  float getBlur(); // blur interface?
  float getResize();
  pixelf getColor();
  
private:
  vec2		truesize;
  vec2		size;
  pixelf	color;
  float		tmin;
  float		tmax;
  float		sigma;
  float		resize;
  Gaussian *	blur;
  
  image<pixelf>	 *G;
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

  enum   D {
    NW = 0, N, NE,
    W, E,
    SW, S, SE
  };
  
  const vec2   dir[8] = {
    {-1, -1}, {0, -1}, {1, -1},
    {-1, 0},           {1, 0},
    {-1, 1},  {0, 1},  {1, 1}
    };
};

#endif /* !CANNY_HPP_ */
