#ifndef VEC3_HPP_
#define VEC3_HPP_

#include "math.hpp"

struct vec3
{
	cordinate	x;
	cordinate	y;
	cordinate	z;

	vec3()
		: x(0), y(0), z(0)
	{}


	vec3(cordinate const & a, cordinate const & b, cordinate const & c)
		: x(a), y(b), z(c)
	{}

	vec3(cordinate * c)
		: x(c[0]), y(c[1]), z(c[2])
	{}
	/*
	template <typename... T>
	vec2(T... c)
	{
		reinterpret_cast<cordinate[3]>(this) = { c... };
	}
	*/
/*	vec3(std::initializer_list<cordinate> const c)
	{
		std::copy(reinterpret_cast<cordinate *>(this),
			reinterpret_cast<cordinate *>(this) + 2,
			c.begin());
	}*/

	vec3(cordinate const & c)
		: x(c), y(c), z(c)
	{}

	vec3(vec3 const & c)
		: x(c.x), y(c.y), z(c.z)
	{}


	vec3 &		operator=(const vec3 & c)
	{
		x = c.x;
		y = c.y;
		z = c.z;
		return *this;
	}

	vec3 		operator-(const vec3 & b) const
	{
		return{ x - b.x, y - b.y, z - b.z };
	}

	vec3 		operator+(const vec3 & b) const
	{
		return{ x + b.x, y + b.y, z + b.z };
	}

	vec3 		operator*(const vec3 & b) const
	{
		return{ x * b.x, y * b.y, z * b.z };
	}

	vec3 		operator/(const vec3 & b) const
	{
		return{ x / b.x, y / b.y, z / b.z };
	}

	vec3 		operator%(const vec3 & b) const
	{
	  return { (cordinate)fmod(x, b.x), (cordinate)fmod(y, b.y), (cordinate)fmod(z, b.z) };
	}

	vec3 & 		operator-=(const vec3 & b)
	{
		x -= b.x;
		y -= b.y;
		z -= b.z;
		return *this;
	}

	vec3 & 		operator+=(const vec3 & b)
	{
		x += b.x;
		y += b.y;
		z += b.z;
		return *this;
	}

	vec3 & 		operator*=(const vec3 & b)
	{
		x *= b.x;
		y *= b.y;
		z *= b.z;
		return *this;
	}

	vec3 & 		operator/=(const vec3 & b)
	{
		x /= b.x;
		y /= b.y;
		z /= b.z;
		return *this;
	}

	vec3 & 		operator%=(const vec3 & b)
	{
	  x = (cordinate)(fmod(x, b.x));
	  y = (cordinate)(fmod(y, b.y));
	  z = (cordinate)(fmod(z, b.z));
	  return *this;
	}


	vec3 		operator-(const cordinate b) const
	{
		return{ x - b, y - b, z - b };
	}

	vec3 		operator+(const cordinate b) const
	{
		return{ x + b, y + b, z + b };
	}

	vec3 		operator*(const cordinate b) const
	{
		return{ x * b, y * b, z * b };
	}

	vec3 		operator/(const cordinate b) const
	{
		return{ x / b, y / b, z / b };
	}

	vec3 		operator%(const cordinate b) const
	{
	  return{ (cordinate)fmod(x, b), (cordinate)fmod(y, b), (cordinate)fmod(z, b) };
	}

	vec3 & 		operator-=(const cordinate b)
	{
		x -= b;
		y -= b;
		z -= b;
		return *this;
	}

	vec3 & 		operator+=(const cordinate b)
	{
		x += b;
		y += b;
		z += b;
		return *this;
	}

	vec3 & 		operator*=(const cordinate b)
	{
		x *= b;
		y *= b;
		z *= b;
		return *this;
	}

	vec3 & 		operator/=(const cordinate b)
	{
		x /= b;
		y /= b;
		z /= b;
		return *this;
	}

	vec3 & 		operator%=(const cordinate b)
	{
		x = fmod(x, b);
		y = fmod(y, b);
		z = fmod(z, b);
		return *this;
	}

	vec3		operator-() const
	{
		return{ -x, -y, -z };
	}

	inline cordinate & operator[](unsigned int i)
	{
		return reinterpret_cast<cordinate *>(this)[i];
	}

	void		clear()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	vec3		cross(const vec3 & b)
	{
		return
		{
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x
		};
	}

	cordinate	dot(const vec3 & b) const
	{
		return { x * b.x + y * b.y + z * b.z };
	}

	cordinate	length() const
	{
		return cordinate(sqrt(x * x + y * y + z * z));
	}

	vec3 &		normalize()
	{
		cordinate len = length();
		if (len != 0)
			*this /= len;
		return *this;
	}

	
  vec3 & 	rotateByCache(double *sin, double *cos)
  {
    vec3	save = *this;

    x = (cordinate)(save.x * (cos[1] * cos[2])
		    + save.y * (-cos[0] * sin[2]
				+ sin[0] * sin[1] * cos[2])
		    + save.z * (sin[0] * sin[2]
				+ cos[0] * sin[1] * cos[2]));
    y = (cordinate)(save.x * (cos[0] * sin[2])
		    + save.y * (cos[0] * cos[2]
				+ sin[0] * sin[1] * sin[2])
		    + save[2] * (-sin[0] * cos[2]
				 + cos[0] * sin[1] * sin[2]));
    z = (cordinate)(save.x * (-sin[0])
		    + save.y * (sin[0] * cos[1])
		    + save.z * (cos[0] * cos[1]));
    return *this;
  }

	vec3 &		rotate(vec3 angle)
	{
	  double radangle[3] = { RAD(angle.x), RAD(angle.y), RAD(angle.z) };
	  double tmpsin[3] = { sin(radangle[0]), sin(radangle[1]), sin(radangle[2]) };
	  double tmpcos[3] = { cos(radangle[0]), cos(radangle[1]), cos(radangle[2]) };
	  return rotateByCache(tmpsin, tmpcos);
	}
};

#endif /* VEC3_HPP_ */
