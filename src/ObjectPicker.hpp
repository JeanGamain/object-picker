#ifndef OBJECTPICKER_HPP_
# define OBJECTPICKER_HPP_

#include "Canny.hpp"
#include "vec2.hpp"
#include "pixel16.hpp"
#include "pixelf.hpp"
#include "image.hpp"
#include "LinearDisplacement.hpp"

class ObjectPicker {
public:
  typedef struct colorsplit_t {
    cordinate		edgePosition;
    vec2		position;
    unsigned int	length;
    pixel16		color[2];
  }			colorSplit;
  
  typedef struct	object_feature_t {
    std::list<pixel16>	sideEdgeColor[2];
    colorSplit *	maxColorSplit;
    object_feature_t() {};
    object_feature_t(int size)
      : maxColorSplit(new colorSplit[size])
    {}
  }			objectFeature;
  
public:
  ObjectPicker(vec2 size);
  ~ObjectPicker();

  void *	detect(image<pixel16> * img);

  bool			isMatchingObjectFeatures(image<pixel16> const & image,
						 objectFeature const & objFeature,
						 cordinate position, char normal, float maxDiff);
  colorSplit		detectColorSplitFeature(image<pixelf> * scany, image<pixel16> * img,
						LinearDisplacement & line);
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
  const char  colorSplitDetetionRay;
  float	      maxPixelDiff;
  
  Canny * canny;
  image<pixelf> * inbw;
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
