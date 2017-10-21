#ifndef OBJECTPICKER_HPP_
# define OBJECTPICKER_HPP_

#include "Canny.hpp"
#include "vec2.hpp"
#include "pixel16.hpp"
#include "pixelf.hpp"
#include "image.hpp"

class ObjectPicker {
public:
  ObjectPicker(vec2 size);
  ~ObjectPicker();

  unsigned int detect(image<pixel16> * img);
  bool	       setLock(unsigned int);

  void		setResize(float r);
 
  float		getResize() const;
  unsigned int	getLock() const;
  
private:
  vec2	truesize;
  float	resize;
  vec2	size;
  
  const unsigned int dump;
  const unsigned int minlength;
  const float tmin;
  const float tmax;
  const float sigma;
 
  Canny * canny;
  image<pixelf> * inbw;
  unsigned int lock;
};

#endif /* ! OBJECTPICKER */
