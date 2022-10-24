#include "Vector.h"

inline float Vec3::operator[] (size_t idx) const
{
	return (&x)[idx];
}

inline float& Vec3::operator[] (size_t idx)
{
	return (&x)[idx];
}

inline Vec3 Vec3::operator*(const float rhs)
{
	return Vec3(x * rhs, y * rhs, z * rhs);
}

inline Vec3 Vec3::operator/(const float rhs)
{
	return Vec3(x / rhs, y / rhs, z / rhs);
}

inline Vec3 Vec3::operator+(const Vec3& rhs)
{
	return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

inline Vec3 Vec3::operator-(const Vec3& rhs)
{
	return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

inline Vec3 Vec3::operator*(const Vec3& rhs)
{
	return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
}

inline Vec3 Vec3::operator/(const Vec3& rhs)
{
	return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
}

inline Vec3& Vec3::operator*=(const float rhs)
{
	this->x *= rhs; this->y *= rhs; this->z *= rhs; return *this;
}

inline Vec3& Vec3::operator/=(const float rhs)
{
	this->x /= rhs; this->y /= rhs; this->z /= rhs;	return *this;
}

inline Vec3& Vec3::operator+=(const Vec3& rhs)
{
	this->x += rhs.x; this->y += rhs.y; this->z += rhs.z; return *this;
}

inline Vec3& Vec3::operator-=(const Vec3& rhs)
{
	this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.z; return *this;
}

inline Vec3& Vec3::operator*=(const Vec3& rhs)
{
	this->x *= rhs.x; this->y *= rhs.y; this->z *= rhs.z; return *this;
}

inline Vec3& Vec3::operator/=(const Vec3& rhs)
{
	this->x /= rhs.x; this->y /= rhs.y; this->z /= rhs.z; return *this;
}
