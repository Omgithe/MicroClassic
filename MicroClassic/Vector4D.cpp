#include "Vector.h"

inline float Vec4::operator[] (size_t idx) const
{
	return (&x)[idx];
}

inline float& Vec4::operator[] (size_t idx)
{
	return (&x)[idx];
}

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