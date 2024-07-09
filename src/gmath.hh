#pragma once
#include "ultratypes.h"

#include <string_view>
#include <cmath>

template<typename T> T sq(T x) { return x * x; }
constexpr f64 toDeg(f64 x) { return x * 180.0 / M_PI; }
constexpr f64 toRad(f64 x) { return x * M_PI / 180.0; }
constexpr f32 toDeg(f32 x) { return x * 180.0f / static_cast<f32>(M_PI); }
constexpr f32 toRad(f32 x) { return x * static_cast<f32>(M_PI) / 180.0f; }

#define COLOR4(hex)                                                                                                    \
    {                                                                                                                  \
        ((hex >> 24) & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 16) & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 8)  & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 0)  & 0xFF) / 255.0f                                                                                  \
    }

#define COLOR3(hex)                                                                                                    \
    {                                                                                                                  \
        ((hex >> 16) & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 8)  & 0xFF) / 255.0f,                                                                                 \
        ((hex >> 0)  & 0xFF) / 255.0f                                                                                  \
    }

union v2;
union v3;
union v4;

union v2
{
    struct {
        f32 x, y;
    };
    f32 e[2];

    v2() = default;
    v2(const v3& v);
    constexpr v2(f32 _x, f32 _y) : x(_x), y(_y) {}
};

union v3
{
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    f32 e[3];

    v3() = default;
    v3(const v2& v);
    v3(const v4& v);
    constexpr v3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}
    constexpr v3(u32 hex)
        : x(((hex >> 16) & 0xFF) / 255.0f),
          y(((hex >> 8)  & 0xFF) / 255.0f),
          z(((hex >> 0)  & 0xFF) / 255.0f) {}

    v3& operator+=(const v3& other);
    v3& operator-=(const v3& other);
    v3& operator*=(const f32 s);
};

union v4
{
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
    /*struct {*/
    /*    v3 v;*/
    /*    f32 s;*/
    /*};*/
    f32 e[4];

    v4() = default;
    constexpr v4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
    /*constexpr v4(v3 _v, f32 _w) : v(_v), s(_w) {}*/
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
    m3(const m4& m) : v{m.v[0], m.v[1], m.v[2]} {}
    constexpr m3(f32 _0, f32 _1, f32 _2, f32 _3, f32 _4, f32 _5, f32 _6, f32 _7, f32 _8)
        : p{_0, _1, _2, _3, _4, _5, _6, _7, _8} {}
};

union qt
{
    struct {
        f32 x, y, z, w;
    };
    /*struct {*/
    /*    v3 v;*/
    /*    f32 s;*/
    /*};*/
    f32 p[4];

    qt(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
    qt(v3 _v, f32 _s) : x(_v.x), y(_v.y), z(_v.z), w(_s) {}
};

#ifdef LOGS
std::string m4ToString(const m4& m, std::string_view prefix);
std::string m3ToString(const m3& m, std::string_view prefix = "");
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
m4 operator*=(m4& l, const m4& r);
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
qt qtAxisAngle(v3 axis, f32 angle);
m4 qtRot(const qt& q);
