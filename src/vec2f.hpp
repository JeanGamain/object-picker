#ifndef VEC2F_HPP_
#define VEC2F_HPP_

#include "math.hpp"
#include "vec2.hpp"

struct vec2f
{
  float	x;
  float	y;

  vec2f()
    : x(0), y(0)
  {}
  

  vec2f(float const & a, float const & b)
    : x(a), y(b)
  {}
  
  vec2f(float * c)
    : x(c[0]), y(c[1])
  {}
  
  vec2f(float const & c)
    : x(c), y(c)
  {}

  vec2f(vec2f const & c)
    : x(c.x), y(c.y)
  {}
  
  vec2f &		operator=(const vec2f & c)
  {
    x = c.x;
    y = c.y;
    return *this;
  }
  
  vec2f 		operator-(const vec2f & b) const
  {
    return{ x - b.x, y - b.y };
  }
  
  vec2f 		operator+(const vec2f & b) const
  {
    return{ x + b.x, y + b.y};
  }

  vec2f 		operator*(const vec2f & b) const
  {
    return{ x * b.x, y * b.y };
  }
  
  vec2f 		operator/(const vec2f & b) const
  {
    return{ x / b.x, y / b.y };
  }
  
  vec2f 		operator%(const vec2f & b) const
  {
    return{ (float)fmod(x, b.x), (float)fmod(y, b.y) };
  }
  
  vec2f & 		operator-=(const vec2f & b)
  {
    x -= b.x;
    y -= b.y;
    return *this;
  }
  
  vec2f & 		operator+=(const vec2f & b)
  {
    x += b.x;
    y += b.y;
    return *this;
  }
  
  vec2f & 		operator*=(const vec2f & b)
  {
    x *= b.x;
    y *= b.y;
    return *this;
  }

  vec2f & 		operator/=(const vec2f & b)
  {
    x /= b.x;
    y /= b.y;
    return *this;
  }
  
  vec2f & 		operator%=(const vec2f & b)
  {
    x = (float)fmod(x, b.x);
    y = (float)fmod(y, b.y);
    return *this;
  }
  
  vec2f 		operator-(const float b) const
  {
    return{ x - b, y - b };
  }
  
  vec2f 		operator+(const float b) const
  {
    return{ x + b, y + b };
  }
  
  vec2f 		operator*(const float b) const
  {
    return{ x * b, y * b };
  }
  
  vec2f 		operator/(const float b) const
  {
    return{ x / b, y / b };
  }
  
  vec2f 		operator%(const float b) const
  {
    return{ (float)fmod(x, b), (float)fmod(y, b) };
  }
  
  vec2f & 		operator-=(const float b)
  {
    x -= b;
    y -= b;
    return *this;
  }
  
  vec2f & 		operator+=(const float b)
  {
    x += b;
    y += b;
    return *this;
  }
  
  vec2f & 		operator*=(const float b)
  {
    x *= b;
    y *= b;
    return *this;
  }
  
  vec2f & 		operator/=(const float b)
  {
    x /= b;
    y /= b;
    return *this;
  }
  
  vec2f & 		operator%=(const float b)
  {
    x = (float)fmod(x, b);
    y = (float)fmod(y, b);
    return *this;
  }

  bool		operator<(const vec2f b) const
  {
    return (x < b.x && y < b.y);
  }
  
  bool		operator>(const vec2f b) const
  {
    return (x > b.x && y > b.y);
  }
  
  bool		operator<=(const vec2f b) const
  {
    return (x <= b.x || y <= b.y);
  }
  
  bool		operator>=(const vec2f b) const
  {
    return (x >= b.x || y >= b.y);
  }
  
  vec2f		operator-() const
  {
    return{ -x, -y };
  }

  bool 		operator==(const vec2f & b) const
  {
    return (x == b.x && y == b.y);
  }

  bool 		operator!=(const vec2f & b) const
  {
    return (x != b.x || y != b.y);
  }
  
  inline float & operator[](unsigned int i)
  {
    return reinterpret_cast<float *>(this)[i];
  }
  
  void		clear()
  {
    x = 0;
    y = 0;
  }
  
  float	dot(const vec2f & b) const
  {
    return { x * b.x + y * b.y };
  }
    
  inline float	to1D(int xlen) const {
    return xlen * y + x;
  }
  
  float	length() const
  {
    return float(sqrt(x * x + y * y));
  }
  
  vec2f &		normalie()
  {
    float len = length();
    if (len != 0)
      *this /= len;
    return *this;
  }
  
  vec2f & 	rotateByCache(double sin, double cos)
  {
    vec2f n = { (float)(x * cos - y * sin), (float)(x * sin + y * cos) };
    *this = n;
    return *this;
  }
  
  vec2f &		rotate(float angle)
  {
    return rotateByCache(sin(RAD(angle)), cos(RAD(angle)));
  }

  vec2	asDec() const
  {
    return vec2((cordinate)x, (cordinate)y);
  }
};

#endif /* VEC2F_HPP_ */
