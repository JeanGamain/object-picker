#ifndef OBJECTPICKER_HPP_
# define OBJECTPICKER_HPP_

#include "Canny.hpp"
#include "vec2.hpp"
#include "pixel16.hpp"
#include "pixelf.hpp"
#include "image.hpp"
#include "LinearDisplacement.hpp"
#include "XrayFeatures.hpp"

class ObjectPicker {
public:
  
  typedef struct	object_feature_t {
    XrayFeatures::xrayFeatures	xraySplits;
    /*OBJECT_feature_t() {};
    object_feature_t(int size)
    : maxColorSplit(new colorSplit[size])*/
  }			objectFeature;
  
public:
  ObjectPicker(vec2 size);
  ~ObjectPicker();

  void *	detect(image<pixel16> * img);

  bool		isMatchingObjectFeatures(image<pixel16> const & image,
					 objectFeature const & objFeature,
					 cordinate position, char normal, float maxDiff);


  objectFeature const &	detectCenterObjectFeature(image<pixelf> * scany, image<pixel16> * img);
 
  void		setResize(float r); 
  float		getResize() const;
  
  bool		setLock(void *);
  void *	getLock() const;
  
private:
  vec2	truesize;
  float	resize;
  vec2	size;
  
  unsigned char * state;
  
  const unsigned int dump;
  const unsigned int minlength;
  const float tmin;
  const float tmax;
  const float sigma;
  const char  colorSplitDetetionRay;
  float	      maxPixelDiff;
  
  XrayFeatures	xrayFeaturesDetector;
  Canny		canny;
  image<pixelf>	inbw;
  objectFeature	lock;
  
public:
  const vec2   dirNormal[2][8] = {
    {
      {-1, 1}, {-1, 0}, {-1, -1},
      {0, -1},           {0, -1},
      {-1, -1}, {-1, 0},  {1, -1}
    },
    {
      {1, -1}, {1, 0}, {1, 1},
      {0, 1},          {0, 1},
      {1, 1},  {1, 0}, {-1, 1}
    }
  };
};

#endif /* ! OBJECTPICKER */
