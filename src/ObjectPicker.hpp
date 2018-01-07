#ifndef OBJECTPICKER_HPP_
# define OBJECTPICKER_HPP_

#include "Canny.hpp"
#include "vec2.hpp"
#include "pixel.hpp"
#include "pixelf.hpp"
#include "image.hpp"
#include "LinearDisplacement.hpp"
#include "XrayFeatures.hpp"

class ObjectPicker {
public:
  
  typedef struct	object_features_t {
    XrayFeatures::xrayFeatures	xray;
    /*OBJECT_feature_t() {};
    object_feature_t(int size)
    : maxColorSplit(new colorSplit[size])*/
  }			objectFeatures;

  typedef struct	object_edges_t {
    std::list<Canny::edge>	outerEdges;
    std::list<Canny::edge>	innerEdges;
  }			objectEdges;
  
public:
  ObjectPicker(vec2 size);
  ~ObjectPicker();

  void *	detect(image<pixel> * img);

private:
  objectFeatures		detectFeatures(image<pixelf> * scany,
					       image<pixel> * img);
  objectEdges			findEdges(objectFeatures const & objectFeatures,
					  image<pixel> * img,
					  unsigned int dump);
  void				render(image<pixel> * img, objectEdges edges);
  void				optimizeDetection(vec2 lastObjectPosition);

public:
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
  objectFeatures	lock;

  vec2		lastObjectPosition;

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
