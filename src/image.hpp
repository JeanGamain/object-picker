#ifndef IMAGE_HPP_
# define IMAGE_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "math.hpp"
#include "vec2.hpp"
#include "pixelf.hpp"

template <class T>
class image {

public:
  vec2		size;
  unsigned int	length;
  T *		pixel;

public:

  image(int width, int height)
    : size(vec2(width, height)), length(width * height), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * length);
    memset(pixel, 0, sizeof(T) * length);
  }
  
  image(vec2 size)
    : size(size),  length(size.x * size.y), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * length);
    memset(pixel, 0, sizeof(T) * length);
  }
  
  image(image<T> const & i)
    : size(i.size),  length(i.length), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * length);  
    memcpy(pixel, i.pixel, sizeof(T) * length);
  }

  image(int width, int height, T *i)
    : size(width, height),  length(width * height), pixel(i)
  {
    // pixel = (T*)malloc(sizeof(T) * width * height);
    //memcpy(pixel, i, sizeof(T) * height * width);
  }

  ~image() {
    free(pixel);
  }
  
  image &	operator=(const image & i) {
    if (&i == this)
      return this;
    length = i.length;
    if (i.size == size) {
      memcpy(pixel, i.pixel, sizeof(T) * size.x * size.y);
    } else {
      size = i.size;
      T* freep = pixel;
      pixel = (T*)realloc(pixel, sizeof(T) * size.x * size.y);
      free(freep);
      memcpy(pixel, i.pixel, size.x * size.y);
    }
    return *this;
  }

  void		set(const image<pixelf> & in) {
    assert(length == in.length);

    for (unsigned int x = 0; x < length; x++) {
      pixel[x].set(in.pixel[x].get());
    }
  }
  
  image &	clear() {
    memset(pixel, 0, sizeof(T) * size.x * size.y);
    return *this;
  }
  
  void	resize(image<T> * const b, float resizex, float resizey) const {
    vec2 s = { cordinate(size.x * resizex), cordinate(size.y * resizey) };
    double sum;
    float fymod;
    float acu;
    float sqr;
    
    assert(b != NULL);
    bool resizeSucess = b->setSize(resizex, resizey);
    assert(resizeSucess);
    assert(b->pixel != NULL && pixel != NULL);
    
    for (int x = 0; x < s.x; ++x) { 
      for (int y = 0; y < s.y; ++y) {
	sum = 0;
	acu = 0;
	for (float x2 = x / resizex; x2 < ((x + 1) / resizex); x2 += 0.96f) { // fix 0.96 to 1
	  fymod = fmod(x2, 1);
	  for (float y2 = y / resizey; y2 < ((y + 1) / resizey); y2 += 0.96f) {
	    sqr = fymod * fmod(y2, 1);
	    sum += this->pixel[int(y2) * size.x + int(x2)].pixel * sqr;
	    acu += sqr;
	  }
	}
	b->pixel[y * s.x + x] = sum / acu;
      }
    }
    /* DUMMY
    for (int x = 0; x < size.x && (i.x = int(x * resizex)) < s.x; x++) {
      for (int y = 0; y < size.y && (i.y = int(y * resizey)) < s.y; y++) {
	b->pixel[i.y * s.x + i.x] = this->pixel[y * size.x + x];
      }
      }*/
    b->size = s;
  }

  image<pixelf>	getHalfResolutionImage() {
    vec2 halfSize = vec2((size.x - size.x % 2) / 2, (size.y - size.y % 2) / 2);
    image<pixelf> *halfScaleImg = new image<pixelf>(halfSize);

    for (int x = 0; x < halfSize.x; ++x) {
      for (int y = 0; y < halfSize.y; ++y) {
	halfScaleImg[y * halfSize.x + x] = (
	  this->pixel[y * halfSize.x + x] +
	  this->pixel[y * halfSize.x + x + 1] +
	  this->pixel[(y + 1) * halfSize.x + x] +
	  this->pixel[(y + 1) * halfSize.x + x + 1]
	  ) / 4;
      }
    }
    
    return halfScaleImg;
  }
  
  bool	setSize(float s) {
    assert(s > 0);
    return setSize(vec2(cordinate(size.x * s), cordinate(size.y * s)));
  }

  bool	setSize(float x, float y) {
    assert(x > 0 && y > 0);
    return setSize(vec2(cordinate(size.x * x), cordinate(size.y * y)));
  }

  bool setSize(vec2 size) {
    assert(size.x > 0 && size.y > 0);
    void * n = realloc(pixel, sizeof(T) * size.x * size.y);
    if (n == NULL)
      return false;
    pixel = (T*)n;
    return true;
  }

  void	getGreyScale(image<pixelf> & out) const {
    assert(out.length == length);
#pragma omp parallel for
    for (unsigned int x = 0; x < length; x++) {
      out.pixel[x].set(pixel[x].get());
    }
  }
};

#endif /* !IMAGE */
