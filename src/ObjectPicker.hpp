#ifndef OBJECTPICKER_HPP_
# define OBJECTPICKER_HPP_

#include "Canny.hpp"
#include "vec2.hpp"
#include "pixel16.hpp"
#include "pixelf.hpp"
#include "image.hpp"

class ObjectPicker {
public:
  typedef struct	object_feature_t {
    std::list<pixel16>	sideEdgeColor[2];
  }			objectFeature;
  
  typedef struct colorsplit_t {
    vec2		position;
    unsigned int	length;
    pixel16		color;		
  }			colorSplit;
  
public:
  ObjectPicker(vec2 size);
  ~ObjectPicker();

  void *		detect(image<pixel16> * img);
  objectFeature const &	detectCenterObjectFeature(image<pixelf> * scany, image<pixel16> * img);
 
  void		setResize(float r); 
  float		getResize() const;
  
  bool		setLock(void *);
  void *	getLock() const;
  
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
  objectFeature	lock;
};

#endif /* ! OBJECTPICKER */
