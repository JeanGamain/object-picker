#ifndef IMAGE_HPP_
# define IMAGE_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "math.hpp"
#include "vec2.hpp"

template <class T>
class image {

public:
  vec2		size;
  T *		pixel;

public:

  image(int width, int height)
    : size(vec2(width, height)), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * width * height);
    memset(pixel, 0, sizeof(T) * width * height);
  }
  
  image(vec2 size)
    : size(size), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * size.x * size.y);
    memset(pixel, 0, sizeof(T) * size.x * size.y);
  }
  
  image(image<T> const & i)
    : size(i.size), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * size.x * size.y);  
    memcpy(pixel, i.pixel, sizeof(T) * size.x * size.y);
  }

  image(int width, int height, T *i)
    : size(width, height), pixel(i)
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

  image &	clear() {
    memset(pixel, 0, sizeof(T) * size.x * size.y);
    return *this;
  }
  
  image &	blend(const image & , float) {
    const char normcolor;
    
  /*  for (int x = 0; i < (i.size.x * i.size.y); x++) {
      pixel[x].setr((image[x].getr() + (i.image[x].getr() * p)) / 2);
      pixel[x].setv((image[x].getv() + (i.image[x].getv() * p)) / 2);
      pixel[x].setb((image[x].getb() + (i.image[x].getb() * p)) / 2);
      }*/
    return *this;
  }

  void		normalize(){
  }

  void	resize(image<T> * const b, float resizex, float resizey) const {
    vec2 s = { cordinate(size.x * resizex), cordinate(size.y * resizey) };
    double sum;
    float fymod;
    float acu;
    float sqr;
    
    assert(b != NULL);
    assert(b->setSize(resizex, resizey));
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
  
};

#endif /* !IMAGE */
