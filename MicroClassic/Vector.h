#pragma once

#include "Define.h"

class Vec2
{
public:
    vec_t x, y;
    //Vec2();
    constexpr Vec2() : x(0.0f), y(0.0f) { }
   // Vec2(vec_t _x, vec_t _y);
    constexpr Vec2(float _x, float _y) : x(_x), y(_y) { }

    float  operator[] (size_t idx) const { return (&x)[idx]; }
    float& operator[] (size_t idx) { return (&x)[idx]; }
};

static inline Vec2 operator*(const Vec2& lhs, const float rhs) { return Vec2(lhs.x * rhs, lhs.y * rhs); }
static inline Vec2 operator/(const Vec2& lhs, const float rhs) { return Vec2(lhs.x / rhs, lhs.y / rhs); }
static inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline Vec2 operator*(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline Vec2 operator/(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static inline Vec2& operator*=(Vec2& lhs, const float rhs) { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline Vec2& operator/=(Vec2& lhs, const float rhs) { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline Vec2& operator+=(Vec2& lhs, const Vec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline Vec2& operator-=(Vec2& lhs, const Vec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline Vec2& operator*=(Vec2& lhs, const Vec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static inline Vec2& operator/=(Vec2& lhs, const Vec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

class Vec3
{
public:
    vec_t x, y, z;
    Vec3();
    Vec3(float _x, float _y, float _z);

    inline float operator[] (size_t idx) const;
    inline float& operator[] (size_t idx);

    inline Vec3 operator*(const float rhs);
    inline Vec3 operator/(const float rhs);
    inline Vec3 operator+(const Vec3& rhs);
    inline Vec3 operator-(const Vec3& rhs);
    inline Vec3 operator*(const Vec3& rhs);
    inline Vec3 operator/(const Vec3& rhs);

    inline Vec3& operator*=(const float rhs);
    inline Vec3& operator/=(const float rhs);
    inline Vec3& operator+=(const Vec3& rhs);
    inline Vec3& operator-=(const Vec3& rhs);
    inline Vec3& operator*=(const Vec3& rhs);
    inline Vec3& operator/=(const Vec3& rhs);

    bool IsValid() const;
    void Random(vec_t minVal, vec_t maxVal);
    void Negate();
    vec_t Length() const;
    vec_t LengthSqr(void) const;
    vec_t DistTo(const Vec3& vOther) const;
    vec_t DistToSqr(const Vec3& vOther) const;
};

class Vec4
{
public:
    vec_t x, y, z, w;
  //  Vec4();
    constexpr Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }
   // Vec4(float _x, float _y, float _z, float _w);
    constexpr Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }

    inline float operator[] (size_t idx) const;
    inline float& operator[] (size_t idx);

    inline Vec4 operator+(const Vec4& rhs);
    inline Vec4 operator-(const Vec4& rhs);
    inline Vec4 operator*(const Vec4& rhs);
};

