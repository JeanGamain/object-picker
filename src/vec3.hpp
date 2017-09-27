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
		return{ fmod(x, b.x), fmod(y, b.y), fmod(z, b.z) };
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
		x = fmod(x, b.x);
		y = fmod(y, b.y);
		z = fmod(z, b.z);
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
		return{ fmod(x, b), fmod(y, b), fmod(z, b) };
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

	
	vec3 & 	rotateByCache(vec3 sin, vec3 cos)
	{
		vec3	save = *this;

		x = (save.x * (cos.y * cos.z)
			+ save.y * (-cos.x * sin.z
			+ sin.x * sin.y * cos.z)
			+ save.z * (sin.x * sin.z
			+ cos.x * sin.y * cos.z));
		y = (save.x * (cos.y * sin.z)
			+ save.y * (cos.x * cos.z
			+ sin.x * sin.y * sin.z)
			+ save.z * (-sin.x * cos.z
			+ cos.x * sin.y * sin.z));
		z = (save.x * (-sin.x)
			+ save.y * (sin.x * cos.y)
			+ save.z * (cos.x * cos.y));
		return *this;
	}

	vec3 &		rotate(vec3 angle)
	{
		vec3	radangle = { RAD(angle.x), RAD(angle.y), RAD(angle.z) };
		vec3	tmpsin = { sin(radangle.x), sin(radangle.y), sin(radangle.z) };
		vec3	tmpcos = { cos(radangle.x), cos(radangle.y), cos(radangle.z) };
		return rotateByCache(tmpsin, tmpcos);
	}
};

#endif /* VEC3_HPP_ */