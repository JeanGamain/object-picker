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
  typedef struct	splitInfo_t {
    vec2	start;
    vec2	end;
    pixel16	sideEdgeColor;
  }			splitInfo;
  
  typedef struct	colorsplit_t {
    pixel16			color;
    unsigned int		colorSum[3];
    unsigned int		length;
    std::list<splitInfo>	mainSplit;
    std::list<splitInfo>	subSection;
  }			colorSplit;

  typedef struct	colorsplitunion_t {
    std::list<colorSplit>	all;
  }			colorSplitUnion;
  
  typedef struct	object_feature_t {
    colorSplitUnion	xraySplits;
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

  void		concatColorSplit(colorSplitUnion * splits,
				 splitInfo split,
				 pixel16 splitColor,
				 unsigned int splitLength,
				 unsigned int colorSum[3]);
  void		finalizeColorSplitUnion(colorSplitUnion * splits, colorSplitUnion * lastSplits);
  bool	        splitLenghtCompare(const colorSplit & a, const colorSplit & b);
  void		detectColorSplitFeature(image<pixelf> * scany, image<pixel16> * img,
					LinearDisplacement & line, colorSplitUnion * lastSplit);

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
  
  Canny		canny;
  image<pixelf>	inbw;
  objectFeature	lock;
  
public:
  vec2		aimPosition;
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
