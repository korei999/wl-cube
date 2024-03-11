#pragma once
#include "ultratypes.h"

#define PI 3.14159265358979323846

#define SQ(X) (X * X)
#define TO_DEG(X) (X * 180.0 / PI)
#define TO_RAD(X) (X * PI / 180.0)

union v2
{
    struct
    {
        f32 x, y;
    };
    f32 e[2];
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
        f32 v2;
        f32 ignoredZ;
    };
    f32 e[3];
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
};

union m4
{
    v4 v[4];
    f32 e[4][4];
    f32 p[16];
};

f32 v3Length(const v3& a);
v3 v3Norm(const v3& a);
v3 v3Cross(const v3& l, const v3& r);
v3 operator-(const v3& l, const v3& r);
v3 operator+(const v3& l, const v3& r);
/* degree(IN RADIANS) between two vectors */
f32 v3Rad(const v3& l, const v3& r);
/* distance between two points in space (vectors) */
f32 v3Dist(const v3& l, const v3& r);
f32 v3Dot(const v3& l, const v3& r);
f32 v4Dot(const v4& l, const v4& r);

m4 m4Identity();
m4 m4Rot(const m4& m, cf32 th, const v3& ax);
m4 m4RotX(const m4& m, cf32 angle);
m4 m4RotY(const m4& m, cf32 angle);
m4 m4RotZ(const m4& m, cf32 angle);
m4 m4Scale(const m4& m, cf32 s);
m4 m4Scale(const m4& m, const v3& s);
m4 m4Trans(const m4& m, const v3& tv);
m4 mat4Pers(cf32 fov, cf32 asp, cf32 n, cf32 f);
m4 mat4Ortho(cf32 l, cf32 r, cf32 b, cf32 t, cf32 n, cf32 f);
m4 m4LookAt(const v3& eyeV, const v3& centerV, const v3& upV);
