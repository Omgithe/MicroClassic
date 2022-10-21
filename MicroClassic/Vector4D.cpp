#include "Vector.h"

constexpr Vec4::Vec4()
	: x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{ }

constexpr Vec4::Vec4(vec_t _x, vec_t _y, vec_t _z, vec_t _w)
	: x(_x), y(_y), z(_z), w(_w)
{ }

inline Vec4 Vec4::operator+(const Vec4& rhs)
{
	return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

inline Vec4 Vec4::operator-(const Vec4& rhs)
{
	return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

inline Vec4 Vec4::operator*(const Vec4& rhs)
{
	return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}