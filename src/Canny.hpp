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

  void			scan(image<pixelf> const & in, image<pixelf> const & out);
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
  float getBlur() const;
  unsigned int getDump() const;
  unsigned int getMinLength() const;
  
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

  const pixelf sobel5X[25] = {
    2, 2, 4, 2, 2,
    1, 1, 2, 1, 1,
    0, 0, 0, 0, 0,
    -1  -1, -2, -1, -1,
    -2, -2, -4, -2, -2,
  };

  const pixelf sobel5Y[25] = {
    -2, -1, 0, 1, 2,
    -2, -1, 0, 1, 2,
    -4, -2, 0, 2, 4,
    -2  -1, 0, 1, 2,
    -2, -1, 0, 1, 2,
  };

  const pixelf scharr3X[9] = {
    3, 10, 2,
    0, 0, 0,
    -3,-10,-3
  };

  const pixelf scharr3Y[9] = {
    3, 0, -3,
    10, 0, -10,
    3, 0, -3
  };
  
  const pixelf sobel3X[9] = {
    1, 2, 1,
    0, 0, 0,
    -1,-2,-1
  };

  const pixelf sobel3Y[9] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
  };
  
  const pixelf * matrix[3][2] =
    {
      { sobel3X, sobel3Y },
      { sobel5X, sobel5Y },
      { scharr3X, scharr3Y },
    };
  
  const int matrixSize[3] = { 3, 5, 3 };
  const int matrixExtreme[3] = { 4, 12, 16 };

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
  const vec2   dirNormal[2][8] = {
    {
      {-1, 1}, {-1, 0}, {-1, -1},
      {0, 1},           {0, -1},
      {-1, -1}, {-1, 0},  {1, -1}
    },
    {
      {1, -1}, {1, 0}, {1, 1},
      {0, -1},          {0, 1},
      {1, 1},  {1, 0}, {-1, 1}
    }
  };
  const unsigned char minEdgeGroupId = 2;
};

#endif /* !CANNY_HPP_ */
