#pragma once
#include "ultratypes.h"

#include <string_view>

#define PI 3.14159265358979323846

#define sq(x) (x * x)
#define toDeg(x) (x * 180.0 / PI)
#define toRad(x) (x * PI / 180.0)

union v4;

union v2
{
    struct
    {
        f32 x, y;
    };
    f32 e[2];

    v2() = default;
    v2(f32 _x, f32 _y) : x(_x), y(_y) {}
};

union v3
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 r, g, b;
    };
    struct
    {
        v2 xy;
        f32 ignoredZ;
    };
    f32 e[3];

    v3() = default;
    v3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}
    v3(const v4& other);

    v3& operator+=(const v3& other);
    v3& operator-=(const v3& other);
    v3& operator*=(const f32 s);
};

union v4
{
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        f32 r, g, b, a;
    };
    struct
    {
        v3 xyz;
        f32 ignoredW;
    };
    struct
    {
        v2 xy;
        v2 ingonredZW;
    };
    f32 e[4];

    v4() = default;
    v4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
};

union m4
{
    v4 v[4];
    f32 e[4][4];
    f32 p[16];
};

union m3
{
    v3 v[4];
    f32 e[3][3];
    f32 p[9];

    m3() = default;
    m3(const m4& m) : v(m.v[0], m.v[1], m.v[2]) {}
    m3(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8) : p(_0, _1, _2, _3, _4, _5, _6, _7, _8) {}
};

#ifdef LOGS
void m4Print(const m4& m, std::string_view prefix = "");
void m3Print(const m3& m, std::string_view prefix = "");
#endif

f32 v3Length(const v3& a);
v3 v3Norm(const v3& a);
v3 v3Norm(const v3& v, const f32 length);
v3 v3Cross(const v3& l, const v3& r);
v3 operator-(const v3& l, const v3& r);
v3 operator+(const v3& l, const v3& r);
v3 operator*(const v3& v, const f32 s);
/* degree(IN RADIANS) between two vectors */
f32 v3Rad(const v3& l, const v3& r);
/* distance between two points in space (vectors) */
f32 v3Dist(const v3& l, const v3& r);
f32 v3Dot(const v3& l, const v3& r);
f32 v4Dot(const v4& l, const v4& r);
m4 m4Iden();
m3 m3Iden();
m4 operator*(const m4& l, const m4& r);
m4 m4Rot(const m4& m, const f32 th, const v3& ax);
m4 m4RotX(const m4& m, const f32 angle);
m4 m4RotY(const m4& m, const f32 angle);
m4 m4RotZ(const m4& m, const f32 angle);
m4 m4Scale(const m4& m, const f32 s);
m4 m4Scale(const m4& m, const v3& s);
m4 m4Translate(const m4& m, const v3& tv);
m4 m4Pers(const f32 fov, const f32 asp, const f32 n, const f32 f);
m4 m4Ortho(const f32 l, const f32 r, const f32 b, const f32 t, const f32 n, const f32 f);
m4 m4LookAt(const v3& eyeV, const v3& centerV, const v3& upV);
m4 m4Transpose(const m4& m);
m3 m3Transpose(const m3& m);
m3 m3Inverse(const m3& m);
m3 m3Normal(const m3& m);
v3 v3Color(const u32 hex);
v4 v4Color(const u32 hex);
