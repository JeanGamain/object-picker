#ifndef XRAYFEATURES_HPP_
# define XRAYFEATURES_HPP_

#include <stack>
#include "image.hpp"
#include "LinearDisplacement.hpp"
#include "pixel16.hpp"
#include "vec2.hpp"
#include "vec2f.hpp"
#include "Canny.hpp"

class XrayFeatures {

public:
  typedef struct	splitInfo_t {
    vec2		pos;
    unsigned int	length;
    pixel16		color;
    bool		start;
    unsigned short	rayId;
    splitInfo_t *	prev;
    splitInfo_t *	next;
  }			splitInfo;
  
  typedef struct	colorsplit_t {
    pixel16			color;
    unsigned int		colorSum[3];
    unsigned int		length;
    unsigned int		nbray;
    unsigned int		score;
    std::list<splitInfo>	split;
  }			colorSplit;

  typedef struct	edge_point2d_t {
    vec2		position;
    bool		normal; // 1 = obj on left side
  }			edgePoint2d;
 
  
  typedef struct	xrayFeatures_t {
    std::list<pixel16>		backgroundColor;
    std::list<pixel16>		objectColor;
    std::list<edgePoint2d>	edges;
  }			xrayFeatures;


public:
  XrayFeatures(vec2 aimTargetPositon,
	       float tmin, float tmax,
	       unsigned int rayc = 17,
	       unsigned int xrayaimwidth = 5);
  ~XrayFeatures();
  
  xrayFeatures const &	detect(image<pixelf> * scany, image<pixel16> * img);
  void			aimTarget(vec2 aimTargetPosition);
  void			setMin(float min);
  void			setMax(float max);
  void			setRayCount(unsigned int rayc);

  
private:
  void			extractFeatures(std::list<colorSplit> & xraySplit, image<pixel16> * img);

  std::list<colorSplit>::iterator	searchBackGroundColors(std::list<colorSplit> & xraySplit,
							       image<pixel16> * img);
  std::list<colorSplit>::iterator	searchObjectColors(std::list<colorSplit> & xraySplit,
							   image<pixel16> * img,
							   std::list<colorSplit> & objSplit,
							   std::list<colorSplit>::iterator & lastBestBackground);
  void					searchObjectEdges(std::list<colorSplit> & objSplit,
							  std::list<colorSplit>::iterator & lastBestObjectSplit);
  std::list<colorSplit>::iterator	splitScoreThresholdSelection(std::list<colorSplit> & splits,
								     std::list<pixel16> & colorOutput,
								     float threshold);
  void		detectColorSplit(image<pixelf> * scany,
				 image<pixel16> * img,
				 LinearDisplacement & line,
				 std::list<colorSplit> & splits,
				 unsigned int rayId);  
  splitInfo *	concatColorSplit(std::list<colorSplit> & splits,
				 splitInfo & split,
				 unsigned int colorSum[3]);
  void		finalizeColorSplitUnion(std::list<colorSplit> & splits,
					std::list<colorSplit> & lastSplits);

private:
  xrayFeatures		features;
  unsigned int		rayCount;
  unsigned int		rayAimWidth;
  unsigned int		aimRatio[3];
  vec2f			baseRayVector;
  vec2			aimPosition;
  vec2			originalAimPosition;
  float			tmin;
  float			tmax;
  float			maxDiff;

  // mem alloc shit
  std::stack<splitInfo *>	availableSplitInfo;
  std::stack<splitInfo *>	useSplitInfo;
  
  // math shit
  float			backGMultiRayBonus;
  unsigned int		minRayPerBackG;
  float			maxBackGThreshold;
  float			objCMultiRayBonus;
  unsigned int		minRayPerObjC;
  float			maxObjCThreshold;
  float			endSplitMalus;
};

#endif /* !XRAYFEATURES_HPP_ */
