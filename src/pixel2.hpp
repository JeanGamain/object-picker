#ifndef PIXEL_HPP_
# define PIXEL_HPP_

#include "math.hpp"

typedef float npixel;

struct pixel
{
public:
  //virtual pixel() = 0;
  //virtual pixel(char const & c) = 0;
  //virtual pixel(char * c) = 0;
  //virtual pixel(pixel const & p) = 0;
  
  //  virtual pixel &	operator=(const pixel & p) = 0;
  //virtual pixel const & operator+(pixel const & p) = 0;
  //virtual pixel const &	operator-(pixel const & p) = 0;

     
  virtual void set(char c) = 0;
  virtual void set(char *c) = 0;
  virtual void get(char *c) const = 0;
  //virtual void set(char *c, const int n) = 0;
  //virtual void get(char *c, const int n) const = 0;
  virtual void clear() = 0;
  virtual void clearColor() = 0;
  
  virtual void setComponent(int i, char c) = 0;
  virtual char getComponent(int i) const = 0;
  virtual void clearComponent(int i) = 0;

  virtual npixel get() const = 0;
 
public:
  //const int	maxComponent;
  //const float	componentMin;
  //const float	componentMax;
};

#endif /* !PIXEL_HPP_ */
