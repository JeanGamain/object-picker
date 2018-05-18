#ifndef PIXELRGBA_HPP_
# define PIXELRGBA_HPP_

#include <cstdint>
#include "math.hpp"
#include "pixel.hpp"
#include "pixelf.hpp"

#include <stdio.h>

const int	maxComponent = 3;
const float	componentMin = 0;
const float	componentMax = 16581375;

struct pixelrgba
{
  typedef union pixel_rgba_u {
    uint32_t		val;
    uint8_t		c[4];
    pixel_rgba_u(uint32_t v)
      : val(v)
    {};
    pixel_rgba_u(uint8_t *v)
      : c{v[0],  v[1], v[2], v[3]}
    {};
    pixel_rgba_u(uint8_t r, uint8_t v, uint8_t b, uint8_t a)
      : c{r, v, b, a}
    {};
    pixel_rgba_u(uint8_t r, uint8_t v, uint8_t b)
      : c{r, v, b, 0}
    {};
  }		pixel_rgba;
   
  pixel_rgba	pixel;
  
  pixelrgba()
    : pixel((unsigned int)0)
  {}
  
  pixelrgba(uint8_t const & p)
    : pixel(p)
  {
  }

  pixelrgba(uint8_t * p)
    : pixel(p)
  {
  }
  
  pixelrgba(unsigned int p)
    : pixel(p)
  {}

  pixelrgba(int p)
    : pixel((uint8_t)p)
  {
  }

  pixelrgba(pixelrgba const & p)
    : pixel(p.pixel.val)
  {}
  
  pixelrgba &		operator=(pixelrgba const & p)
  {
    pixel.val = p.pixel.val;
    return *this;
  }

  pixelrgba const &	operator-(pixelrgba const & b)
  {
    pixel.c[0] -= b.pixel.c[0];
    pixel.c[1] -= b.pixel.c[1];
    pixel.c[2] -= b.pixel.c[2];
    return *this;
  }
  
  pixelrgba const &	operator+(pixelrgba const & b)
  {
    pixel.c[0] += b.pixel.c[0];
    pixel.c[1] += b.pixel.c[1];
    pixel.c[2] += b.pixel.c[2];
    return *this;
  }

  float diff(pixelrgba const & b) const { // use inline
    return (float)(abs(int(this->pixel.c[0]) - int(b.pixel.c[0]) +
		       int(this->pixel.c[1]) - int(b.pixel.c[1]) +
		       int(this->pixel.c[2]) - int(b.pixel.c[2]))) / 3 / 255 * 100;
  }
  
  npixel get() const {
    return (npixel)(pixel.c[0] + pixel.c[1] + pixel.c[2]) / 3;
    /*if (p > 255) {
      printf("NONN %f\n", (npixel)(p));
      }*/
    //return (npixel)(pixel.c[0]);
    /*return (npixel)(
		    morton3d((uint32_t)pixel.c[0], (uint32_t)pixel.c[1], (uint32_t)pixel.c[2])
		    / componentMax * 255
		    );*/
  }

  void getrvb(uint8_t *p) {
    p[0] = pixel.c[0];
    p[1] = pixel.c[1];
    p[2] = pixel.c[2];
  }

  void getrvba(uint8_t *p) {
    p[0] = pixel.c[0];
    p[1] = pixel.c[1];
    p[2] = pixel.c[2];
    p[3] = pixel.c[3];
  }

  uint8_t getComponent(unsigned int i) const {
    return pixel.c[i % 4];
  }

  uint8_t getr() const {
    return pixel.c[0];
  }
  
  uint8_t getv() const {
    return pixel.c[1];
  }
  
  uint8_t getb() const {
    return pixel.c[2];
  }

  void set(uint8_t rvba) {
    pixel.c[0] = rvba;
    pixel.c[1] = rvba;
    pixel.c[2] = rvba;
  }

  void set(uint32_t rvba) {
    pixel_rgba b(rvba);

    pixel.c[0] = b.c[0];
    pixel.c[1] = b.c[1];
    pixel.c[2] = b.c[2];
  }
  
  void setColor(uint8_t rvba) {
    pixel.c[0] = rvba;
    pixel.c[1] = rvba;
    pixel.c[2] = rvba;
  }

  void setComponent(int i, uint8_t c) {
    pixel.c[i % 4] = c;
  }

  void setrvb(uint8_t r, uint8_t v, uint8_t b) {
    pixel.c[0] = r;
    pixel.c[1] = v;
    pixel.c[2] = b;
  }
  
  void setr(uint8_t c) {
    pixel.c[0] = c;
  }
  
  void setv(uint8_t c) {
    pixel.c[1] = c;
  }
  
  void setb(uint8_t c) {
    pixel.c[2] = c;
  }

  void clear() {
    pixel.val = 0;
  }

  void clearColor() {
    pixel.c[0] = 0;
    pixel.c[1] = 0;
    pixel.c[2] = 0;
  }

  void clearComponent(unsigned int i) {
    pixel.c[i % 4] = 0;
  }

  void clrr() {
    pixel.c[0] = 0;
  }

  void clrv() {
    pixel.c[1] = 0;
  }

  void clrb() {
    pixel.c[2] = 0;
  }
};

#endif /* !PIXELRGBA_HPP_ */
