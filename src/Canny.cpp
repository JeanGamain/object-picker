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
    blur((sigma > 0) ? new Gaussian(sigma) : NULL)
{
  rawEdgeBuffer = new rawEdgePixel[size.x * size.y];
  nms = new image<pixelf>(size);
  boundClearScan = new unsigned char[size.x * size.y];

  // ????
  nms->clear();
  memset(boundClearScan, 0, sizeof(char) * size.x * size.y);
  for (int x = 0; x < size.x; ++x) {
      boundClearScan[x] = 255;
      rawEdgeBuffer[x].g = 0;
      boundClearScan[(size.y - 1) * size.x + x] = 255;
      rawEdgeBuffer[(size.y - 1) * size.x + x].g = 0;
  }
  int sx = size.x - 1;
  for (int y = 0; y < size.y; ++y) {
      boundClearScan[y * size.x] = 255;
      rawEdgeBuffer[y * size.x].g = 0;
      boundClearScan[y * size.x + sx] = 255;
      rawEdgeBuffer[y * size.x + sx].g = 0;
  }
  // Reuse array
  edges = new int[size.x * size.y];
  edgeList = new std::list<edge>();

  PARMVSVAR(1, 200, 1, &this->tmax, "canny max");
  PARMVSVAR(1, 200, 1, &this->tmin, "canny min");
}

Canny::~Canny() {
  delete(rawEdgeBuffer);
  delete(nms);
  delete(blur);
  delete(boundClearScan);
  delete(edges);
  delete(edgeList);
}


inline void	Canny::axisConvolution(image<pixelf> const & in, const cordinate pos,
				       rawEdgePixel & out)
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
  out.g = (npixel)hypot(Gx, Gy);
  out.axis = (npixel)atan2f(Gx, Gy);
}


image<pixelf> *	Canny::scan(image<pixelf> const & in)
{
  assert(in.pixel != NULL);
  assert(in.size == size);

  // clear output
  if (blur != NULL) {
    memcpy(nms->pixel, in.pixel, in.length * sizeof(pixelf));
    blur->filter(in.size, in.pixel, nms->pixel);
    if (renderMode == 7) {
      for (int x = 1; x < in.size.x - 1; x++) {
	for (int y = 1; y < in.size.y - 1; y++) {
	  img->pixel[in.size.x * y + x].set((uint8_t)nms->pixel[in.size.x * y + x].pixel);
	}
      }
      //memcpy(img->pixel, nms->pixel, in.length * sizeof(pixelf));
      return NULL;
    }
  }
 
  #pragma omp parallel for
  for (int x = 1; x < in.size.x - 1; x++) {
    for (int y = 1; y < in.size.y - 1; y++) {
      axisConvolution(*nms, in.size.x * y + x, rawEdgeBuffer[in.size.x * y + x]);
    }
  }
  #pragma omp parallel for
  for (int x = 1; x < in.size.x - 1; x++) {
    for (int y = 1; y < in.size.y - 1; y++) {
      const int c = in.size.x * y + x;

      int nord = c - in.size.x;
      int sud = c + in.size.x;
      
      float eDir = (npixel)(fmodf(rawEdgeBuffer[c].axis + M_PI, M_PI) / M_PI * 8);
      if (
	  ((eDir > 3 && eDir <= 5)// 90°
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[nord].g// N
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[sud].g) ||// S

	  ((eDir <= 1 || eDir > 7)// 0°
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[c - 1].g// W
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[c + 1].g) ||// E

	  ((eDir > 1 && eDir <= 3)// 45°
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[nord + 1].g// NE
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[sud - 1].g) ||// SW

	  ((eDir > 5 && eDir <= 7)// 135°
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[nord - 1].g// NW
	   && rawEdgeBuffer[c].g > rawEdgeBuffer[sud + 1].g)// SE
	  )
	nms->pixel[c] = rawEdgeBuffer[c].g;
      else
	nms->pixel[c] = 0;
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

  if (nms->pixel[position] < tmin || detectionState[position] > 0)
    return false;
  nedges = 1;
  newedge.position = position;
  newedge.length = 1;
  newedge.color = 0; // use for pixel moy of the edge 
  newedge.point = new std::list<edgePoint>();
  detectionState[position] = edgeGroupId;
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
	/*if (!vec2(pos1d % size.x, pos1d / size.x).in(vec2(0, 0), img->size)) {
	 printf("NOO\n");
	 image.pixel[pos1d].setrvb(255, 0, 0);
	 continue;
	}*/
	detectionState[pos1d] = edgeGroupId;
	edges[nedges] = pos1d;
	nedges++;
	newedge.length++;
	if (dump == 0 || newedge.length % dump == 0) {
	  if (renderMode == 4 || renderMode == 5 || renderMode == 6) {
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
	  //image.pixel[(dirNormal[start][dk] * 1).to1D(image.size.x) + pos1d].setrvb(0, 255, 0);
	  //image.pixel[(dirNormal[!start][dk] * 1).to1D(image.size.x) + pos1d].setrvb(0, 0, 255);
	  for (i = features.backgroundColor.begin();
	       i != features.backgroundColor.end() && innerColor < 0;
	       ++i) {
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
		 ++i);
	    if (i != features.objectColor.end()) {
	      image.pixel[pos1d].setrvb(255, 255, 0);      
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
  if (edgeGroupId == 255)
    edgeGroupId = Canny::minEdgeGroupId;
  else
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



const pixelf Canny::sobel3[2][9] = {
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
