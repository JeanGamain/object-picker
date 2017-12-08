#ifndef XRAYFEATURES_HPP_
# define XRAYFEATURES_HPP_

#include "image.hpp"
#include "LinearDisplacement.hpp"
#include "pixel16.hpp"
#include "vec2.hpp"

class XrayFeatures {

public:
  typedef struct	splitInfo_t {
    vec2		pos;
    unsigned int	length;
    pixel16		color;
    splitInfo_t *	prev;
    splitInfo_t *	next;
  }			splitInfo;
  
  typedef struct	colorsplit_t {
    pixel16			color;
    unsigned int		colorSum[3];
    unsigned int		length;
    unsigned int		nbray;
    unsigned int		score;
    std::list<splitInfo *>	split;
  }			colorSplit;

  typedef struct	xrayFeatures_t {
    std::list<colorSplit>	all;
  }			xrayFeatures;


public:
  XrayFeatures(vec2 aimTargetPositon,
	       float tmin, float tmax,
	       unsigned int rayc = 17,
	       unsigned int xrayaimwidth = 5);
  ~XrayFeatures();
  
  xrayFeatures const &	detect(image<pixelf> * scany, image<pixel16> * img);
  void			aimTarget(vec2 aimTargetPosition);
  void	setMin(float min);
  void	setMax(float max);
  void	setRayCount(unsigned int rayc);

  
private:
  void		extractFeatures();
  void		concatColorSplit(xrayFeatures * splits,
				 splitInfo split,
				 pixel16 splitColor,
				 unsigned int splitLength,
				 unsigned int colorSum[3]);
  
  void		concatColorSplit(xrayFeatures * splits,
				 splitInfo * split,
				 unsigned int colorSum[3]);
  void		finalizeColorSplitUnion(xrayFeatures * splits, xrayFeatures * lastSplits);
  bool	        splitLenghtCompare(const colorSplit & a, const colorSplit & b);
  void		detectColorSplitFeatures(image<pixelf> * scany,
					 image<pixel16> * img,
					 LinearDisplacement & line,
					 xrayFeatures * lastSplit);  

private:
  xrayFeatures		features;
  unsigned int		rayCount;
  unsigned int		rayAimWidth;
  vec2			baseRayVector;
  vec2			aimPosition;
  vec2			originalAimPosition;
  float			tmin;
  float			tmax;
};

#endif /* !XRAYFEATURES_HPP_ */
