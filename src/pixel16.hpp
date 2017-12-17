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
  
  pixel16(char const & p)
    : pixel(0)
  {
    isetrvb(p, p, p);
  }

  pixel16(char * p)
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
    isetrvb((char)p, (char)p, (char)p);
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
    char Brgb[3];

    b.get(Brgb);
    Brgb[0] -= this->getr();
    Brgb[1] -= this->getv();
    Brgb[2] -= this->getb();
    this->set(Brgb);
    return *this;
  }
  pixel16 const &	operator+(pixel16 const & b)
  {
    char Brgb[3];

    b.get(Brgb);
    Brgb[0] += this->getr();
    Brgb[1] += this->getv();
    Brgb[2] += this->getb();
    this->set(Brgb);
    return *this;
  }
  

  void set(float rvb) {
    rvb /= 255.0;
    this->pixel = (uint16_t(rvb * 0x1f) & 0x1f) | ((uint16_t(rvb * 0x1f) & 0x1f) << 11) | ((uint16_t(rvb * 0x3f) & 0x3f) << 5); // 11 befor 5 to erase 6 of <<5
  }
  
  void set(char rvb) {
    uint16_t p = rvb & 0x3f;
    this->pixel = p | (p << 11) | (p << 5);
  }
  
  void set(char *p) {
    pixel = 0;
    pixel |= p[0] & 0x001f;
    pixel |= p[1] & 0x001f << 5;
    pixel |= p[2] & 0x001f << 11;
  }

  
  void get(char *p) const {
    p[0] = static_cast<char>(this->pixel & 0x001f);
    p[1] = static_cast<char>(this->pixel & 0x03e0 >> 5);
    p[2] = static_cast<char>(this->pixel & 0xf800 >> 11);
  }

  void clear() {
    this->pixel = 0;
  }

  void clearColor() {
    this->pixel = 0;
  }
  /*
  void setComponent(int i, char c) {
    void (pixel16::*setCptr[3])(char) = { // make const
      &pixel16::setr,
      &pixel16::setv,
      &pixel16::setb
    };
    // check i;
    (this->*setCptr[i])(c);
  }
  
  char getComponent(int i) const {
    char (pixel16::*getCptr[3])() const = {
      &pixel16::getr,
      &pixel16::getv,
      &pixel16::getb
    };
    // check i;
    return (this->*getCptr[i])();
  }


  void clearComponent(int i) {
    void (pixel16::* clrCptr[3])() = {
      &pixel16::clrr,
      &pixel16::clrv,
      &pixel16::clrb
    };
    // check i;
    (this->*clrCptr[i])();
    }*/

  npixel get() const {
    /*
    return (npixel)(
		    (
		     double(this->pixel & 0x1f) / 0x1f +
		     double(this->pixel >> 5 & 0x3f) / 0x3f +
		     double(this->pixel >> 11 & 0x1f) / 0x1f
		     ) / 3 * 255);*/
    return (npixel)(double(this->pixel) / 65025 * 255);
  }

  npixel getRnorm() const {
    return (npixel)(float(this->pixel & 0x1f) / 0x1f * 255);
  }

  npixel getGnorm() const {
    return (npixel)(float(this->pixel >> 5 & 0x3f) / 0x3f * 255);
  }

  npixel getBnorm() const {
    return (npixel)(float(this->pixel >> 11 & 0x1f) / 0x1f * 255);
  }

  char getr() const {
    return static_cast<char>(this->pixel & 0x001f);
  }
  
  char getv() const {
    return static_cast<char>(this->pixel >> 5 & 0x003f);
  }
  
  char getb() const {
    return static_cast<char>(this->pixel >> 11 & 0x001f);
  }

  float diff(pixel16 const & b) const { // use inline
    return (float(abs(int(this->pixel & 0x1f) - (b.pixel & 0x1f))) / 0x1f +
	    float(abs(int(this->pixel >> 5 & 0x3f) - (b.pixel >> 5 & 0x3f))) / 0x3f +
	    float(abs(int(this->pixel >> 11 & 0x1f) - (b.pixel >> 11 & 0x1f))) / 0x1f) / 3 * 100;
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
  
  void setrvb(char r, char v, char b) {
    pixel = 0;
    pixel |= r & 0x1f;
    pixel |= v & 0x3f << 5;
    pixel |= b & 0x1f << 11;
  }

  void setrvb(char *p) {
    pixel = 0;
    pixel |= p[0] & 0x001f;
    pixel |= p[0] & 0x001f << 5;
    pixel |= p[0] & 0x001f << 11;
  }

  void fsetrvb(char r, char v, char b) {
    pixel = 0;
    pixel |= r;
    pixel |= v << 5;
    pixel |= b << 11;
  }

  void fsetrvb(char *p) {
    pixel = 0;
    pixel |= p[0];
    pixel |= p[0] << 5;
    pixel |= p[0] << 11;
  }

  void isetrvb(char r, char v, char b) {
    pixel |= r & 0x001f;
    pixel |= v & 0x001f << 5;
    pixel |= b & 0x001f << 11;
  }

  void isetrvb(char *p) {
    pixel |= p[0] & 0x001f;
    pixel |= p[0] & 0x001f << 5;
    pixel |= p[0] & 0x001f << 11;
  }
  
  void setr(char c) {
    pixel &= 0xfe0;
    pixel |= c & 0x001f;
  }
  
  void setv(char c) {
    pixel &= 0xfc1f;
    pixel |= c & 0x001f << 5;
  }
  
  void setb(char c) {
    pixel &= 0x7ff;
    pixel |= c & 0x001f << 11;
  }
};

#endif /* !PIXEL16_HPP_ */
