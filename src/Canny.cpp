#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <cmath>
#include <list>
#include "pixelf.hpp"
#include "Convolution.hpp"
#include "Gaussian.hpp"
#include "math.hpp"
#include "pixel.hpp"

#include "visualdebug.hpp"
extern image<pixel> * img;
extern unsigned int renderMode;

Canny::Canny(vec2 const & size,
	     unsigned char * state,
	     unsigned int dump = 45,
	     unsigned int minlength = 6,
	     const float tmin = 50,
	     const float tmax = 60,
	     const float sigma = 1.4f)
  : size(size),
    edgeGroupId(Canny::minEdgeGroupId),
    detectionState(state),
    dump(dump),
    minlength(minlength),
    tmin(tmin),
    tmax(tmax),
    sigma(sigma),
    blur((sigma > 0) ? new Gaussian(sigma) : NULL),
    matrixIdx(0)
{
  G = new image<pixelf>(size);
  Gx = new image<pixelf>(size);
  Gy = new image<pixelf>(size);
  nms = new image<pixelf>(size);
  boundClearScan = new unsigned char[size.x * size.y];
 
  G->clear();
  Gx->clear();
  Gy->clear();
  nms->clear();
  memset(boundClearScan, 0, sizeof(char) * size.x * size.y);
  for (int x = 0; x < size.x; ++x) {
      boundClearScan[x] = 255;
      boundClearScan[(size.y - 1) * size.x + x] = 255;
  }
  int sx = size.x - 1;
  for (int y = 0; y < size.y; ++y) {
      boundClearScan[y * size.x] = 255;
      boundClearScan[y * size.x + sx] = 255;
  }
  // Reuse array
  edges = (int*)Gx->pixel;
  edgeList = new std::list<edge>();

  PARMVSVAR(1, 200, 1, &this->tmax, "canny max");
  PARMVSVAR(1, 200, 1, &this->tmin, "canny min");
  PARMVSVAR(0, 2, 1, &this->matrixIdx, "canny matrixIdx");
}

Canny::~Canny() {
  delete(G);
  delete(Gx);
  delete(Gy);
  delete(nms);
  delete(blur);
  delete(boundClearScan);
  delete(edgeList);
}


inline void	Canny::axisConvolution(image<pixelf> const & in, cordinate pos,
				       pixelf * g, pixelf * dir)
{
  cordinate	kdir[8];
  
  kdir[0] = pos + in.size.x;
  kdir[1] = pos - in.size.x;
  kdir[2] = pos + 1;
  kdir[3] = pos - 1;
  kdir[4] = kdir[0] + 1;
  kdir[5] = kdir[0] - 1;
  kdir[6] = kdir[1] + 1;
  kdir[7] = kdir[1] - 1;
  /*
    7 1 6
    3   2
    5 0 4
  */
  double Gxy =
    in.pixel[kdir[6]] * sobel3[0][2] +
    in.pixel[kdir[5]] * sobel3[0][6];
  double Gx = Gxy +
    in.pixel[kdir[7]] * sobel3[0][0] + in.pixel[kdir[1]] * sobel3[0][1] +
    in.pixel[kdir[0]] * sobel3[0][7] + in.pixel[kdir[4]] * sobel3[0][8];
  double Gy = Gxy +
    in.pixel[kdir[7]] * sobel3[1][0] + in.pixel[kdir[3]] * sobel3[1][3] +
    in.pixel[kdir[2]] * sobel3[1][5] + in.pixel[kdir[4]] * sobel3[1][8];
  
  // calc angle
  //G->pixel[x].set(ABS(Gx->pixel[x].get() + Gy->pixel[x].get()));
  g[pos] = (npixel)(hypot(Gx, Gy));
  dir[pos] = (npixel)(fmodf(atan2f(Gx, Gy) + M_PI, M_PI) / M_PI * 8);
}


image<pixelf> *	Canny::scan(image<pixelf> const & in)
{
  /*
  const vec2 vecDirAssoc[] {
    
  }*/
    
  assert(in.pixel != NULL);
  assert(in.size == size);

  // clear output
  if (blur != NULL) {
    memcpy(nms->pixel, in.pixel, in.length * sizeof(pixelf));
    blur->filter(in.size, in.pixel, nms->pixel);
  }
  
  #pragma omp parallel for
  for (int x = 1; x < in.size.x - 1; x++) {
    for (int y = 1; y < in.size.y - 1; y++) {
      axisConvolution(*nms, in.size.x * y + x, G->pixel, Gx->pixel);      
    }
  }
  #pragma omp parallel for
  for (int x = 1; x < in.size.x - 1; x++) {
    for (int y = 1; y < in.size.y - 1; y++) {
      const int c = in.size.x * y + x;

      int nord = c - in.size.x;
      int sud = c + in.size.x;
      
      float eDir = Gx->pixel[c].pixel;
      if (
	  ((eDir > 3 && eDir <= 5)// 90°
	   && G->pixel[c] > G->pixel[nord]// N
	   && G->pixel[c] > G->pixel[sud]) ||// S

	  ((eDir <= 1 || eDir > 7)// 0°
	   && G->pixel[c] > G->pixel[c - 1]// W
	   && G->pixel[c] > G->pixel[c + 1]) ||// E

	  ((eDir > 1 && eDir <= 3)// 45°
	   && G->pixel[c] > G->pixel[nord + 1]// NE
	   && G->pixel[c] > G->pixel[sud - 1]) ||// SW

	  ((eDir > 5 && eDir <= 7)// 135°
	   && G->pixel[c] > G->pixel[nord - 1]// NW
	   && G->pixel[c] > G->pixel[sud + 1])// SE
	  )
	nms->pixel[c] = G->pixel[c];
      else
	nms->pixel[c] = 0;
      /*
      if (
	  ((eDir > 3 && eDir <= 5)			// 90°
	   && G->pixel[c] > G->pixel[nord]		// N
	   && G->pixel[c] > G->pixel[sud])){		// S
	kdir = vec2(3, 4);
      }
      if ((eDir <= 1 || eDir > 7)			// 0°
	   && G->pixel[c] > G->pixel[c - 1]		// W
	  && G->pixel[c] > G->pixel[c + 1]) {		// E
	kdir = vec2(1, 6);
      }
      if ((eDir > 1 && eDir <= 3)			// 45°
	  && G->pixel[c] > G->pixel[nord + 1]		// NE
	  && G->pixel[c] > G->pixel[sud - 1]) {		// SW
	kdir = vec2(2, 5);
      }
      if ((eDir > 5 && eDir <= 7) 			// 135°
	  && G->pixel[c] > G->pixel[nord - 1]		// NW
	  && G->pixel[c] > G->pixel[sud + 1]) {		// SE
	kdir = vec2(0, 7);
      }
      int cDir = (int)(round(Gx->pixel[c].pixel)) % 7;
      if (G->pixel[c] > G->pixel[c + dir[cDir].to1D(in.size.x)] &&
	  G->pixel[c] > G->pixel[c + (-dir[cDir]).to1D(in.size.x)])
	nms->pixel[c] = G->pixel[c];
      else {
	if (kdir.x != -1) {
	  printf("FUCK %lu %lu -> %d\n", kdir.x, kdir.y, cDir);
	}
	nms->pixel[c] = 0;
	}*/
    }
  }
  return nms;
}

bool		Canny::getEdge(edge & newedge,
			       cordinate position,
			       unsigned int dump,
			       XrayFeatures::xrayFeatures const & features,
			       image<pixel> const & image,
			       float maxPixelDiff,
			       bool start) {
  const vec2	halfImgSize = image.size / 2;
  unsigned int	nedges;
  cordinate	kdir[9];
  cordinate	pos1d;

  // nms->pixel[position] < tmax ||
  if (detectionState[position] > 0)
    return false;
  nedges = 1;
  newedge.position = position;
  newedge.length = 1;
  newedge.color = 0; // use for pixel moy of the edge 
  newedge.point = new std::list<edgePoint>();
  detectionState[position] = edgeGroupId;
  image.pixel[position].set((uint32_t)((uint32_t)16581375 / 100 * edgeGroupId + 10) ^ 1);
  edges[0] = position;
  do {
    kdir[8] = edges[--nedges];
    kdir[0] = kdir[8] + size.x;
    kdir[1] = kdir[8] - size.x;
    kdir[2] = kdir[8] + 1;
    kdir[3] = kdir[8] - 1;
    kdir[4] = kdir[0] + 1;
    kdir[5] = kdir[0] - 1;
    kdir[6] = kdir[1] + 1;
    kdir[7] = kdir[1] - 1;
    for (int k = 0; k < 8; k++) {
      pos1d = kdir[k];
      if (nms->pixel[pos1d] >= tmin
	  && detectionState[pos1d] < 1) {
	//if (pos1d < (size.x * 1 + 1) || pos1d >= ((size.x - 1) + (size.y - 1) * size.x)) {
	// printf("NOO\n");
	// continue;
	// }
	detectionState[pos1d] = edgeGroupId;
	edges[nedges] = pos1d;
	nedges++;
	newedge.length++;
	if (dump == 0 || newedge.length % dump == 0) {
	  if (renderMode == 4 || renderMode == 5) {
	    image.pixel[pos1d].setrvb(0, 0, 255);
	    continue;
	  }

	  int dk;
	  vec2 pos(pos1d % image.size.x, pos1d / image.size.x);
	  
	  if (k == 1 || k == 6) {
	    dk = (pos.x < halfImgSize.x) ? 3 : 4;
	  } else if (k == 3 || k == 4) {
	    dk = (pos.y < halfImgSize.y) ? 1 : 6;
          } else {
	    dk =
	      (pos.x < halfImgSize.x) ?
	      (pos.y < halfImgSize.y ? 0 : 5) :
	      (pos.y < halfImgSize.y ? 2 : 7);
	  }
	  std::list<pixel>::const_iterator i;
	  int innerColor = -1;
	  pixel edgeColor[2] = {
	    image.pixel[(dirNormal[start][dk] * 1).to1D(image.size.x) + pos1d],
	    image.pixel[(dirNormal[!start][dk] * 1).to1D(image.size.x) + pos1d]
	  };
	  image.pixel[(dirNormal[start][dk] * 1).to1D(image.size.x) + pos1d].setrvb(0, 255, 0);
	  image.pixel[(dirNormal[!start][dk] * 1).to1D(image.size.x) + pos1d].setrvb(0, 0, 255);
	  for (i = features.backgroundColor.begin();
	       i != features.backgroundColor.end() && innerColor < 0;
	       i++) {
	    if ((*i).diff(edgeColor[0]) <= maxPixelDiff)
	      innerColor = 1;
	    else if ((*i).diff(edgeColor[1]) <= maxPixelDiff)
	      innerColor = 0;
	  }
	  image.pixel[pos1d].setrvb(255, 0, 0);
	  //	  image.pixel[pos1d].set((uint32_t)((uint32_t)16581375 / 100 * edgeGroupId + 10) ^ 1);
	  if (innerColor > -1) {
	    for (i = features.objectColor.begin();
		 i != features.objectColor.end() && (*i).diff(edgeColor[innerColor]) > maxPixelDiff;
		 i++);
	    if (i != features.objectColor.end()) {
	      newedge.point->push_front({ pos1d, (char)dk });
	    } else {
	      continue;
	    }
	  } else {
	    continue;
	  }
	}
      }
    }
  } while (nedges > 0);
  edgeGroupId++;
  return true;
}

void				Canny::clearState() {
  memcpy(detectionState, boundClearScan, size.x * size.y * sizeof(char));
  edgeGroupId = Canny::minEdgeGroupId;
}

void Canny::setMax(float max) {
  tmax = max;
}

void Canny::setMin(float min) {
  tmin = min;
}

void Canny::setBlur(float s) {
  sigma = s;
  if (blur != NULL)
    delete(blur);
  blur = NULL;
  if (s > 0) {
    blur = new Gaussian(sigma);
  }
}

void Canny::setDump(unsigned int d) {
  dump = d;
}

void Canny::setMinLength(unsigned int l) {
  minlength = l;
}

float	Canny::getMax() const {
  return tmax;
}

float	Canny::getMin() const {
  return tmin;
}

float	Canny::getBlur() const {
  return sigma;
}

unsigned int Canny::getDump() const {
  return dump;
}

unsigned int Canny::getMinLength() const {
  return minlength;
}

const pixelf Canny::sobel3[2][9] =
  {
    {
      1, 2, 1,
      0, 0, 0,
      -1,-2,-1
    },
    {
      -1, 0, 1,
      -2, 0, 2,
      -1, 0, 1
    }
  };

const vec2	Canny::dir[8] = {
  {-1, -1}, {0, -1}, {1, -1},
  {-1, 0},           {1, 0},
  {-1, 1},  {0, 1},  {1, 1}
};

const vec2	Canny::dirNormal[2][8] = {
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

const unsigned char	Canny::minEdgeGroupId = 2;

/*
const pixelf Canny::sobel5[2][25] =
  {
    {
      2, 2, 4, 2, 2,
      1, 1, 2, 1, 1,
      0, 0, 0, 0, 0,
      -1  -1, -2, -1, -1,
      -2, -2, -4, -2, -2,
    },
    {
      -2, -1, 0, 1, 2,
      -2, -1, 0, 1, 2,
      -4, -2, 0, 2, 4,
      -2  -1, 0, 1, 2,
      -2, -1, 0, 1, 2,
    }
  };

const pixelf Canny::scharr3[2][9] =
  {
    {
      3, 10, 2,
      0, 0, 0,
      -3,-10,-3
    },
    {
      3, 0, -3,
      10, 0, -10,
      3, 0, -3
    }
  };
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
