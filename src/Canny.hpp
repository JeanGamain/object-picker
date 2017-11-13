#ifndef CANNY_HPP_
# define CANNY_HPP_

#include <stdint.h>
#include <list>
#include "pixelf.hpp"
#include "image.hpp"
#include "math.hpp"
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

  image<pixelf> *	scan(image<pixelf> * in);
  bool			getEdge(edge & newedge, cordinate position, unsigned int dump);
  void			clearState();
  std::list<edge> *	get();

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
  
  const pixelf GMx[9] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
  };
  
  
  const pixelf GMy[9] = {
    1, 2, 1,
    0, 0, 0,
    -1,-2,-1
  };

public:
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
};

#endif /* !CANNY_HPP_ */
