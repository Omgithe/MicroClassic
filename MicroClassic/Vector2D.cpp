#include "Vector.h"

constexpr Vec2::Vec2() : x(0.0f), y(0.0f) {}

constexpr Vec2::Vec2(vec_t _x, vec_t _y) : x(_x), y(_y) {}

float Vec2::operator[] (size_t idx) const
{
	return (&x)[idx];
}

float& Vec2::operator[] (size_t idx)
{
	return (&x)[idx];
}

inline Vec2 Vec2::operator*(const vec_t rhs)
{
	return Vec2(x * rhs, y * rhs); 
}

inline Vec2 Vec2::operator/(const vec_t rhs)
{
	return Vec2(x / rhs, y / rhs);
}

inline Vec2 Vec2::operator+(const Vec2& rhs)
{
	return Vec2(x + rhs.x, y + rhs.y);
}

inline Vec2 Vec2::operator-(const Vec2& rhs)
{
	return Vec2(x - rhs.x, y - rhs.y);
}

inline Vec2 Vec2::operator*(const Vec2& rhs)
{
	return Vec2(x * rhs.x, y * rhs.y);
}

inline Vec2 Vec2::operator/(const Vec2& rhs)
{
	return Vec2(x / rhs.x, y / rhs.y);
}

inline Vec2& Vec2::operator*=(const vec_t rhs)
{
	this->x *= rhs; this->y *= rhs;	return *this;
}

inline Vec2& Vec2::operator/=(const vec_t rhs)
{
	this->x /= rhs; this->y /= rhs;	return *this;
}

inline Vec2& Vec2::operator+=(const Vec2& rhs)
{
	this->x += rhs.x; this->y += rhs.y;	return *this;
}

inline Vec2& Vec2::operator-=(const Vec2& rhs)
{
	this->x -= rhs.x; this->y -= rhs.y;	return *this;
}

inline Vec2& Vec2::operator*=(const Vec2& rhs)
{
	this->x *= rhs.x; this->y *= rhs.y;	return *this;
}

inline Vec2& Vec2::operator/=(const Vec2& rhs)
{
	this->x /= rhs.x; this->y /= rhs.y;	return *this;
}
