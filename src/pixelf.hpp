#ifndef PIXELF_HPP_
# define PIXELF_HPP_

#include "math.hpp"
#include "pixel.hpp"

struct pixelf: public pixel
{
public:
  float pixel;
  
  pixelf()
    : pixel(0.0)
  {}
  
  pixelf(char const & p)
    : pixel(p)
  {
  }
  
  pixelf(char * p)
    : pixel(*p)
  {
  }
  
  
  pixelf(float p)
    : pixel(p)
  {}
  
  pixelf(int p)
    : pixel(p)
  {
  }
  
  
  pixelf(pixelf const & p)
    : pixel(p.pixel)
  {}
  
  pixelf &		operator=(const pixelf & p)
  {
  this->pixel = p.pixel;
  return *this;
}
  
  bool			operator>(pixelf const & b)
  {
  return (this->pixel > b.pixel);
}
  
  bool	 		operator==(pixelf const & b)
  {
  return (this->pixel == b.pixel);
}
  
  bool			operator>=(pixelf const & b)
  {
  return (this->pixel >= b.pixel);
  }

  pixelf const &	operator-(pixelf const & b)
  {
    this->pixel -= b.pixel;
    return *this;
  }
  
  pixelf const &	operator+(pixelf const & b)
  {
    this->pixel += b.pixel;
    return *this;
  }

  pixelf const &	operator+(double const & b)
  {
  this->pixel += (float)b;
    return *this;
  }

  void set(char c) {
    this->pixel = c;
  }
  
  void set(char *c) {
    this->pixel = *c;
  }

  void get(char *p) const {
    p[0] = static_cast<char>(this->pixel);
  }

  void clear() {
    this->pixel = 0;
  }

  void clearColor() {
    this->pixel = 0;
  }

  void setComponent(int i, char c) {
    if (i == 0) {
      this->pixel = c;
    }
  }
  
  char getComponent(int i) const {
    return (i == 0) ? static_cast<char>(this->pixel) : 0;
  }

  void clearComponent(int i) {
    if (i == 0) {
      this->pixel = 0; // componentMin
    }
  }

  npixel get() const {
    return this->pixel;
  }
};

#endif /* !PIXELF_HPP_ */
