#ifndef PIXEL16_HPP_
# define PIXEL16_HPP_

#include "math.hpp"
#include "pixel.hpp"

struct pixel16: public pixel
{
  int16_t pixel;
  
  const int	maxComponent = 3;
  const float	componentMin = -32768;
  const float	componentMax = 32768;

  
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


  
  pixel16 &		operator=(const pixel16 & p)
  {
    this.pixel = p.pixel;
    return *this;
  }

  pixel16 const &	operator-(pixel const & b)
  {
    char Brvb[3];

    b.get(Brgb, 3);
    Brvb[0] -= this.getr();
    Brvb[1] -= this.getv();
    Brvb[2] -= this.getb();
    this.set(Brvb, 3);
    return *this;
  }
  
  pixel16 const &	operator+(pixel16 const & b) const
  {
    char Brvb[3];

    b.get(Brgb, 3);
    Brvb[0] += this.getr();
    Brvb[1] += this.getv();
    Brvb[2] += this.getb();
    this.set(Brvb, 3);
    return *this;
  }


  void set(char rvb) {
    uint16_t p = rvb & 0x1f;
    pixel = p | (p << 5) | (p << 11);    
  }
  
  void set(char *p) {
    pixel = 0;
    pixel |= p[0] & 0x001f;
    pixel |= p[1] & 0x001f << 5;
    pixel |= p[2] & 0x001f << 11;
  }

  
  void get(char *p) {
    p[0] = static_cast<char>(this.pixel & 0x001f);
    p[1] = static_cast<char>(this.pixel & 0x07e0 >> 5);
    p[2] = static_cast<char>(this.pixel & 0xf800 >> 11);
  }

  void clear() {
    this.pixel = 0;
  }

  void clearColor() {
    this.pixel = 0;
  }

  void setComponent(int i, char c) {
    const void (pixel16::* setCptr)(char c)[3] = {
      this.setr(),
      this.setv(),
      this.setb()
    };
    // check i;
    setCptr[i](c);
  }
  
  char getComponent(int i) const {
    const char (pixel16::* getCptr)()[3] = {
      this.getr(),
      this.getv(),
      this.getb()
    };
    // check i;
    return getCptr[i](c);
  }


  void clearComponent(int i) {
    const void (pixel16::* cltCptr)(char c)[3] = {
      this.clrr(),
      this.clrv(),
      this.clrb()
    };
    // check i;
    clrCptr[i]();
  }

  npixel getNorme() const {
    return NORM((npixel)((this.pixel & 0x001f) + (this.pixel & 0x07e0 >> 5) + (this.pixel & 0xf800 >> 11)) / 3, this.componentMin, this.componentMax);
  }

  
  char getr() {
    return static_cast<char>(this.pixel & 0x001f);
  }
  
  char getv() {
    return static_cast<char>(this.pixel & 0x07e0 >> 5);
  }
  
  char getb() {
    return static_cast<char>(this.pixel & 0xf800 >> 11);
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
    pixel |= r & 0x001f;
    pixel |= v & 0x001f << 5;
    pixel |= b & 0x001f << 11;
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
