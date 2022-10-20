#pragma once

#include "Define.h"

class Vec2
{
public:
    vec_t x, y;
    constexpr Vec2();
    constexpr Vec2(vec_t _x, vec_t _y);

    float operator[] (size_t idx) const;
    float& operator[] (size_t idx);

    inline Vec2 operator*(const vec_t rhs);
    inline Vec2 operator/(const vec_t rhs);
    inline Vec2 operator+(const Vec2& rhs);
    inline Vec2 operator-(const Vec2& rhs);
    inline Vec2 operator*(const Vec2& rhs);
    inline Vec2 operator/(const Vec2& rhs);

    inline Vec2& operator*=(const vec_t rhs);
    inline Vec2& operator/=(const vec_t rhs);
    inline Vec2& operator+=(const Vec2& rhs);
    inline Vec2& operator-=(const Vec2& rhs);
    inline Vec2& operator*=(const Vec2& rhs);
    inline Vec2& operator/=(const Vec2& rhs);
};

class Vec3
{
public:
    vec_t x, y, z;
    constexpr Vec3();
    constexpr Vec3(float _x, float _y, float _z);

    float operator[] (size_t idx) const;
    float& operator[] (size_t idx);

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



