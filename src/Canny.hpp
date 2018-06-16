#ifndef CANNY_HPP_
# define CANNY_HPP_

#include <list>
#include "pixelf.hpp"
#include "pixel.hpp"
#include "image.hpp"
#include "math.hpp"
#include "XrayFeatures.hpp"
#include "Gaussian.hpp"

class Canny {
public:
  typedef struct	edge_point_t {
    cordinate		position;
    char		normal;
  }			edgePoint;
  
  typedef struct		edge_t {
    cordinate			position;
    unsigned int		length;
    unsigned char		color;		
    std::list<edgePoint> *	point;
  }				edge;
  
public:
  Canny(vec2 const & size, unsigned char * state, unsigned int dump, unsigned int minlength, const float tmin, const float tmax, const float sigma);
  ~Canny();

  image<pixelf> *	scan(image<pixelf> const & in);
  bool			getEdge(edge & newedge,
				cordinate position,
				unsigned int dump,
				XrayFeatures::xrayFeatures const & features,
				image<pixel> const & image,
				float maxPixelDiff,
				bool start);
  void			clearState();

  // use generic image type

  void setMax(float max);
  void setMin(float min);
  void setBlur(float s);
  void setDump(unsigned int d);
  void setMinLength(unsigned int l);
  float getMax() const;
  float getMin() const;
  float	getBlur() const;
  unsigned int getDump() const;
  unsigned int getMinLength() const;

private:
  static void	axisConvolution(image<pixelf> const & in, cordinate pos,
				pixelf * g, pixelf * dir);
  
private:
  vec2		size;
  unsigned char	edgeGroupId;
  unsigned char * detectionState;
  unsigned int	dump;
  unsigned int	minlength;
  float		tmin;
  float		tmax;
  float		sigma;
  Gaussian *	blur;
  
  image<pixelf>	* G;
  image<pixelf> * Gx;
  image<pixelf>	* Gy;
  image<pixelf> * nms;
  std::list<edge> * edgeList;
  unsigned char * boundClearScan;
  int *		  edges;

public:
  int		matrixIdx;

  static const pixelf sobel3[2][9];

  enum   D {
    NW = 0, N, NE,
    W, E,
    SW, S, SE
  };
  
  static const vec2   dir[8];
  /*
  const vec2   dirNormal[2][8] = {
    {
      {-1, 1}, {1, 0}, {-1, -1},
      {0, -1},           {0, -1},
      {-1, -1}, {1, 0},  {-1, 1}
    },
    {
      {1, -1}, {-1, 0}, {1, 1},
      {0, 1},          {0, 1},
      {1, 1},  {-1, 0}, {1, -1}
    }
  };
  */ 
  static const vec2   dirNormal[2][8];
  static const unsigned char minEdgeGroupId;
};

#endif /* !CANNY_HPP_ */
