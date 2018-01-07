#ifndef PIXEL16_HPP_
# define PIXEL16_HPP_

#include "math.hpp"
#include "pixelf.hpp"
#include <stdio.h>

struct pixel16
{
public:
  uint16_t pixel;
  
  pixel16()
    : pixel(0)
  {}
  
  pixel16(uint8_t const & p)
    : pixel(0)
  {
    isetrvb(p, p, p);
  }

  pixel16(uint8_t * p)
    : pixel(0)
  {
    isetrvb(p[0], p[1], p[2]);
  }

  
  pixel16(uint16_t p)
    : pixel(p)
  {}

  pixel16(int p)
    : pixel(0)
  {
    isetrvb((uint8_t)p, (uint8_t)p, (uint8_t)p);
  }
  
  
  pixel16(pixel16 const & p)
    : pixel(p.pixel)
  {}

  bool	 		operator==(pixel16 const & b)
  {
    return (this->get() == b.get());
  }
  
  pixel16 &		operator=(const pixel16 & p)
  {
    this->pixel = p.pixel;
    return *this;
  }

  pixel16 const &	operator-(pixel16 const & b)
  {
    uint8_t Brgb[3];

    b.get(Brgb);
    Brgb[0] -= this->getr();
    Brgb[1] -= this->getv();
    Brgb[2] -= this->getb();
    this->set(Brgb);
    return *this;
  }
  pixel16 const &	operator+(pixel16 const & b)
  {
    uint8_t Brgb[3];

    b.get(Brgb);
    Brgb[0] += this->getr();
    Brgb[1] += this->getv();
    Brgb[2] += this->getb();
    this->set(Brgb);
    return *this;
  }
  
  float diff(pixel16 const & b) const { // use inline
    return (float(abs(int(this->pixel & 0x1f) - (b.pixel & 0x1f))) / 0x1f +
	    float(abs(int(this->pixel >> 5 & 0x3f) - (b.pixel >> 5 & 0x3f))) / 0x3f +
	    float(abs(int(this->pixel >> 11 & 0x1f) - (b.pixel >> 11 & 0x1f))) / 0x1f) / 3 * 100;
  }
  
  
  npixel getRCP() const {
    
    return (npixel)(
		    (
		     double(this->pixel & 0x1f) / 0x1f +
		     double(this->pixel >> 5 & 0x3f) / 0x3f +
		     double(this->pixel >> 11 & 0x1f) / 0x1f
		     ) / 3 * 255);
  }

  npixel get() const {
    return (npixel)(double(this->pixel) / 65025 * 255);
  }
  
  void get(uint8_t *p) const {
    p[0] = static_cast<uint8_t>(this->pixel & 0x001f);
    p[1] = static_cast<uint8_t>(this->pixel & 0x03e0 >> 5);
    p[2] = static_cast<uint8_t>(this->pixel & 0xf800 >> 11);
  }
  
  /*  
  uint8_t getComponent(int i) const {
    uint8_t (pixel16::*getCptr[3])() const = {
      &pixel16::getr,
      &pixel16::getv,
      &pixel16::getb
    };
    // check i;
    return (this->*getCptr[i])();
  }
  */

  uint8_t getr() const {
    return static_cast<uint8_t>(this->pixel & 0x001f);
  }
  
  uint8_t getv() const {
    return static_cast<uint8_t>(this->pixel >> 5 & 0x003f);
  }
  
  uint8_t getb() const {
    return static_cast<uint8_t>(this->pixel >> 11 & 0x001f);
  }

  void set(float rvb) {
    rvb /= 255.0;
    this->pixel = (uint16_t(rvb * 0x1f) & 0x1f) | ((uint16_t(rvb * 0x1f) & 0x1f) << 11) | ((uint16_t(rvb * 0x3f) & 0x3f) << 5); // 11 befor 5 to erase 6 of <<5
  }
  
  void set(uint8_t rvb) {
    uint16_t p = rvb & 0x3f;
    this->pixel = p | (p << 11) | (p << 5);
  }
  
  void set(uint8_t *p) {
    pixel = 0;
    pixel |= p[0] & 0x001f;
    pixel |= p[1] & 0x001f << 5;
    pixel |= p[2] & 0x001f << 11;
  }
  
  void setrvb(uint8_t r, uint8_t v, uint8_t b) {
    pixel = 0;
    pixel |= r & 0x1f;
    pixel |= v & 0x3f << 5;
    pixel |= b & 0x1f << 11;
  }

  void fsetrvb(uint8_t r, uint8_t v, uint8_t b) {
    pixel = 0;
    pixel |= r;
    pixel |= v << 5;
    pixel |= b << 11;
  }

  void fsetrvb(uint8_t *p) {
    pixel = 0;
    pixel |= p[0];
    pixel |= p[0] << 5;
    pixel |= p[0] << 11;
  }

  void isetrvb(uint8_t r, uint8_t v, uint8_t b) {
    pixel |= r & 0x001f;
    pixel |= v & 0x001f << 5;
    pixel |= b & 0x001f << 11;
  }

  void isetrvb(uint8_t *p) {
    pixel |= p[0] & 0x001f;
    pixel |= p[0] & 0x001f << 5;
    pixel |= p[0] & 0x001f << 11;
  }

  /*
  void setComponent(int i, uint8_t c) {
    void (pixel16::*setCptr[3])(uint8_t) = { // make const
      &pixel16::setr,
      &pixel16::setv,
      &pixel16::setb
    };
    // check i;
    (this->*setCptr[i])(c);
  }
  */
  void setr(uint8_t c) {
    pixel &= 0xfe0;
    pixel |= c & 0x001f;
  }
  
  void setv(uint8_t c) {
    pixel &= 0xfc1f;
    pixel |= c & 0x001f << 5;
  }
  
  void setb(uint8_t c) {
    pixel &= 0x7ff;
    pixel |= c & 0x001f << 11;
  }

  /*
  void clearComponent(int i) {
    void (pixel16::* clrCptr[3])() = {
      &pixel16::clrr,
      &pixel16::clrv,
      &pixel16::clrb
    };
    // check i;
    (this->*clrCptr[i])();
    }*/

  void clear() {
    this->pixel = 0;
  }

  void clearColor() {
    this->pixel = 0;
  }

  void clrr() {
    pixel &= 0xfe0;
  }

  void clrv() {
    pixel &= 0xfc1f;
  }

  void clrb() {
    pixel &= 0x7ff;
  }
};

#endif /* !PIXEL16_HPP_ */
