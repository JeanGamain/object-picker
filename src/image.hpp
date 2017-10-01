#ifndef IMAGE_HPP_
# define IMAGE_HPP_

#include <stdlib.h>
#include <stdio.h>
#include "math.hpp"
#include "pixel.hpp"
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
  }
  
  image(vec2 size)
    : size(size), pixel(NULL)
  {
    pixel = (T*)malloc(sizeof(T) * size.x * size.y);
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

  virtual ~image() {
    free(pixel);
  }
  
  image &	operator=(const image & i) {
    if (i.size == size) {
      memcpy(pixel, i.pixel, sizeof(T) * size.x * size.y);
    } else {
      size = i.size;
      pixel = (T*)realloc(pixel, sizeof(T) * size.x * size.y);
      memcpy(pixel, i.pixel, size.x * size.y);
    }
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
  
};

#endif /* !IMAGE */
