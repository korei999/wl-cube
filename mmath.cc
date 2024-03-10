#include "headers/mmath.hh"
#include <cmath>

static void mat4LookAtInternal(m4 dest, cv3 Right, cv3 Up, cv3 Down, cv3 Point);

void
vec3Add(v3 d, cv3 s)
{
    d[0] += s[0];
    d[1] += s[1];
    d[2] += s[2];
}

void
vec3Add3(v3 d, cv3 s0, cv3 s1)
{
    d[0] = s0[0] + s1[0];
    d[1] = s0[1] + s1[1];
    d[2] = s0[2] + s1[2];
}

void
vec3Sub(v3 d, cv3 s)
{
    d[0] -= s[0];
    d[1] -= s[1];
    d[2] -= s[2];
}

void
vec3Sub3(v3 d, cv3 s0, cv3 s1)
{
    d[0] = s0[0] - s1[0];
    d[1] = s0[1] - s1[1];
    d[2] = s0[2] - s1[2];
}

f32
vec3Length(cv3 s)
{
    f32 res = 0;

    res += SQ(s[0]);
    res += SQ(s[1]);
    res += SQ(s[2]);

    return sqrt(res);
}

void
vec3Norm(v3 s)
{
    f32 length = vec3Length(s);

    s[0] /= length;
    s[1] /= length;
    s[2] /= length;
}

void
vec3Norm2(v3 d, cv3 s)
{
    f32 length = vec3Length(s);

    d[0] = s[0] / length;
    d[1] = s[1] / length;
    d[2] = s[2] / length;
}

void
vec3Scale(v3 d, cf32 scalar)
{
    d[0] *= scalar;
    d[1] *= scalar;
    d[2] *= scalar;
}

void
vec3Scale3(v3 d, cv3 s, cf32 scalar)
{
    d[0] = s[0] * scalar;
    d[1] = s[1] * scalar;
    d[2] = s[2] * scalar;
}

f32
vec3Dot(cv3 s0, cv3 s1)
{
    return s0[0] * s1[0] + s0[1] * s1[1] + s0[2] * s1[2];
}

f32
vecRad(cv3 s0, cv3 s1)
{
    return acos(vec3Dot(s0, s1) / (vec3Length(s0) * vec3Length(s1)));
}

f64
vecDist(cv3 s0, cv3 s1)
{
    return sqrt(SQ(s1[0] - s0[0]) + SQ(s1[1] - s0[1]) + SQ(s1[2] - s0[2]));
}

f64
vecDistCompare(v3 s0, v3 s1)
{
    s32 cmp = 0;
    f32 t0 = vecDist(s0, s1);

    return cmp;
}

f64
vecDistCmpDec(cv3 s0, cv3 s1, cv3 eye)
{
    return vecDist(s0, eye) - vecDist(s1, eye);
}

f64
vecDistCmpInc(cv3 s0, cv3 s1, cv3 eye)
{
    return vecDist(s1, eye) - vecDist(s0, eye);
}

void
vecSortByDistToEye(v3 v[], cv3 vEye, f64 (*FuncCmp)(cv3 s0, cv3 s1, cv3 eye), cu64 size)
{
    /* start from 1 compare with previous positions */
    for (u64 i = 1; i < size; i++)
    {
        v3 key;
        VEC3_COPY(key, v[i]);

        s64 j = i - 1;
        /* j has to be signed */
        while (j >= 0 && FuncCmp(v[j], key, vEye) < 0)
        {
            VEC3_COPY(v[j + 1], v[j]);
            j--;
        }
        VEC3_COPY(v[j + 1], key);
    }
}

void
vecCross(v3 d, cv3 s)
{
    v3 temp;

    temp[0] = d[1] * s[2] - s[1] * d[2];
    temp[1] = d[2] * s[0] - s[2] * d[0];
    temp[2] = d[0] * s[1] - s[0] * d[1];

    VEC3_COPY(d, temp);
}

void
vecCross3(v3 d, cv3 s0, cv3 s1)
{
    d[0] = s0[1] * s1[2] - s1[1] * s0[2];
    d[1] = s0[2] * s1[0] - s1[2] * s0[0];
    d[2] = s0[0] * s1[1] - s1[0] * s0[1];
}

void
mat4Add(m4 d, cm4 s)
{
    for (u32 i = 0; i < 4; i++)
        for (u32 j = 0; j < 4; j++)
            d[i][j] += s[i][j];
}

void
mat4Sub(m4 d, cm4 s)
{
    for (u32 i = 0; i < 4; i++)
        for (u32 j = 0; j < 4; j++)
            d[i][j] -= s[i][j];
}

void
mat4Scale(m4 d, cf32 scalar)
{
    m4 sMat {
        {scalar, 0,      0,      0},
        {0,      scalar, 0,      0},
        {0,      0,      scalar, 0},
        {0,      0,      0,      1}
    };

    mat4Mul(d, sMat);
}

void
mat4ScaleV(m4 d, cv3 vScalar)
{
    m4 sMat {
        {vScalar[0], 0,          0,          0},
        {0,          vScalar[1], 0,          0},
        {0,          0,          vScalar[2], 0},
        {0,          0,          0,          1}
    };

    mat4Mul(d, sMat);
}

void
mat4Trans(m4 d, cv3 s)
{
    m4 trMat {
        {1,    0,    0,    0},
        {0,    1,    0,    0},
        {0,    0,    1,    0},
        {s[0], s[1], s[2], 1}
    };

    mat4Mul(d, trMat);
}

void
mat4Mul(m4 d, cm4 s)
{
    m4 t;

    t[0][0] = (d[0][0] * s[0][0]) + (d[1][0] * s[0][1]) + (d[2][0] * s[0][2]) + (d[3][0] * s[0][3]);
    t[0][1] = (d[0][1] * s[0][0]) + (d[1][1] * s[0][1]) + (d[2][1] * s[0][2]) + (d[3][1] * s[0][3]);
    t[0][2] = (d[0][2] * s[0][0]) + (d[1][2] * s[0][1]) + (d[2][2] * s[0][2]) + (d[3][2] * s[0][3]);
    t[0][3] = (d[0][3] * s[0][0]) + (d[1][3] * s[0][1]) + (d[2][3] * s[0][2]) + (d[3][3] * s[0][3]);

    t[1][0] = (d[0][0] * s[1][0]) + (d[1][0] * s[1][1]) + (d[2][0] * s[1][2]) + (d[3][0] * s[1][3]);
    t[1][1] = (d[0][1] * s[1][0]) + (d[1][1] * s[1][1]) + (d[2][1] * s[1][2]) + (d[3][1] * s[1][3]);
    t[1][2] = (d[0][2] * s[1][0]) + (d[1][2] * s[1][1]) + (d[2][2] * s[1][2]) + (d[3][2] * s[1][3]);
    t[1][3] = (d[0][3] * s[1][0]) + (d[1][3] * s[1][1]) + (d[2][3] * s[1][2]) + (d[3][3] * s[1][3]);

    t[2][0] = (d[0][0] * s[2][0]) + (d[1][0] * s[2][1]) + (d[2][0] * s[2][2]) + (d[3][0] * s[2][3]);
    t[2][1] = (d[0][1] * s[2][0]) + (d[1][1] * s[2][1]) + (d[2][1] * s[2][2]) + (d[3][1] * s[2][3]);
    t[2][2] = (d[0][2] * s[2][0]) + (d[1][2] * s[2][1]) + (d[2][2] * s[2][2]) + (d[3][2] * s[2][3]);
    t[2][3] = (d[0][3] * s[2][0]) + (d[1][3] * s[2][1]) + (d[2][3] * s[2][2]) + (d[3][3] * s[2][3]);

    t[3][0] = (d[0][0] * s[3][0]) + (d[1][0] * s[3][1]) + (d[2][0] * s[3][2]) + (d[3][0] * s[3][3]);
    t[3][1] = (d[0][1] * s[3][0]) + (d[1][1] * s[3][1]) + (d[2][1] * s[3][2]) + (d[3][1] * s[3][3]);
    t[3][2] = (d[0][2] * s[3][0]) + (d[1][2] * s[3][1]) + (d[2][2] * s[3][2]) + (d[3][2] * s[3][3]);
    t[3][3] = (d[0][3] * s[3][0]) + (d[1][3] * s[3][1]) + (d[2][3] * s[3][2]) + (d[3][3] * s[3][3]);

    MAT4_COPY(d, t);
}

void
mat4Mul3(m4 d, cm4 s0, cm4 s1)
{
    d[0][0] = (s0[0][0] * s1[0][0]) + (s0[1][0] * s1[0][1]) + (s0[2][0] * s1[0][2]) + (s0[3][0] * s1[0][3]);
    d[0][1] = (s0[0][1] * s1[0][0]) + (s0[1][1] * s1[0][1]) + (s0[2][1] * s1[0][2]) + (s0[3][1] * s1[0][3]);
    d[0][2] = (s0[0][2] * s1[0][0]) + (s0[1][2] * s1[0][1]) + (s0[2][2] * s1[0][2]) + (s0[3][2] * s1[0][3]);
    d[0][3] = (s0[0][3] * s1[0][0]) + (s0[1][3] * s1[0][1]) + (s0[2][3] * s1[0][2]) + (s0[3][3] * s1[0][3]);

    d[1][0] = (s0[0][0] * s1[1][0]) + (s0[1][0] * s1[1][1]) + (s0[2][0] * s1[1][2]) + (s0[3][0] * s1[1][3]);
    d[1][1] = (s0[0][1] * s1[1][0]) + (s0[1][1] * s1[1][1]) + (s0[2][1] * s1[1][2]) + (s0[3][1] * s1[1][3]);
    d[1][2] = (s0[0][2] * s1[1][0]) + (s0[1][2] * s1[1][1]) + (s0[2][2] * s1[1][2]) + (s0[3][2] * s1[1][3]);
    d[1][3] = (s0[0][3] * s1[1][0]) + (s0[1][3] * s1[1][1]) + (s0[2][3] * s1[1][2]) + (s0[3][3] * s1[1][3]);

    d[2][0] = (s0[0][0] * s1[2][0]) + (s0[1][0] * s1[2][1]) + (s0[2][0] * s1[2][2]) + (s0[3][0] * s1[2][3]);
    d[2][1] = (s0[0][1] * s1[2][0]) + (s0[1][1] * s1[2][1]) + (s0[2][1] * s1[2][2]) + (s0[3][1] * s1[2][3]);
    d[2][2] = (s0[0][2] * s1[2][0]) + (s0[1][2] * s1[2][1]) + (s0[2][2] * s1[2][2]) + (s0[3][2] * s1[2][3]);
    d[2][3] = (s0[0][3] * s1[2][0]) + (s0[1][3] * s1[2][1]) + (s0[2][3] * s1[2][2]) + (s0[3][3] * s1[2][3]);

    d[3][0] = (s0[0][0] * s1[3][0]) + (s0[1][0] * s1[3][1]) + (s0[2][0] * s1[3][2]) + (s0[3][0] * s1[3][3]);
    d[3][1] = (s0[0][1] * s1[3][0]) + (s0[1][1] * s1[3][1]) + (s0[2][1] * s1[3][2]) + (s0[3][1] * s1[3][3]);
    d[3][2] = (s0[0][2] * s1[3][0]) + (s0[1][2] * s1[3][1]) + (s0[2][2] * s1[3][2]) + (s0[3][2] * s1[3][3]);
    d[3][3] = (s0[0][3] * s1[3][0]) + (s0[1][3] * s1[3][1]) + (s0[2][3] * s1[3][2]) + (s0[3][3] * s1[3][3]);
}

void
mat4Rotx(m4 d, cf32 angle)
{
    m4 Xaxis {
        {1, 0,           0,          0},
        {0, cos(angle),  sin(angle), 0},
        {0, -sin(angle), cos(angle), 0},
        {0, 0,           0,          1}
    };

    mat4Mul(d, Xaxis);
}

void
mat4Roty(m4 d, cf32 angle)
{
    m4 Yaxis {
        {cos(angle), 0, -sin(angle), 0},
        {0,          1, 0,           0},
        {sin(angle), 0, cos(angle),  0},
        {0,          0, 0,           1}
    };

    mat4Mul(d, Yaxis);
}

void
mat4Rotz(m4 d, cf32 angle)
{
    m4 Zaxis {
        {cos(angle),  sin(angle), 0, 0},
        {-sin(angle), cos(angle), 0, 0},
        {0,           0,          1, 0},
        {0,           0,          0, 1}
    };

    mat4Mul(d, Zaxis);
}

void
mat4Rot(m4 d, cf32 th, cv3 ax)
{
    cf32 c = cos(th);
    cf32 s = sin(th);

    cf32 x = ax[0];
    cf32 y = ax[1];
    cf32 z = ax[2];

    m4 r {
        {((1 - c) * SQ(x)) + c,     ((1 - c) * x * y) - s * z, ((1 - c) * x * z) + s * y, 0},
        {((1 - c) * x * y) + s * z, ((1 - c) * SQ(y)) + c,     ((1 - c) * y * z) - s * x, 0},
        {((1 - c) * x * z) - s * y, ((1 - c) * y * z) + s * x, ((1 - c) * SQ(z)) + c,     0},
        {0,                         0,                         0,                         1}
    };

    mat4Mul(d, r);
}

void
mat4Pers(m4 d, cf32 fov, cf32 asp, cf32 n, cf32 f)
{
    /*
     *	m4 pers = {
     *		{n / r,     0,       0,                     0},
     *		{0,         n / t,   0,                     0},
     *		{0,         0,      -(f + n) / (f - n),    -1},
     *		{0,         0,      -(2 * f * n) / (f - n), 0}
     *	};
     */

    /* b(back), l(left) are not needed if our viewing volume is symmetric */
    f32 t = n * tan(fov / 2);
    f32 r = t * asp;

    d[0][0] = n / r;
    d[0][1] = 0;
    d[0][2] = 0;
    d[0][3] = 0;

    d[1][0] = 0;
    d[1][1] = n / t;
    d[1][2] = 0;
    d[1][3] = 0;

    d[2][0] = 0;
    d[2][1] = 0;
    d[2][2] = -(f + n) / (f - n);
    d[2][3] = -1;

    d[3][0] = 0;
    d[3][1] = 0;
    d[3][2] = -(2*f*n) / (f - n);
    d[3][1] = 0;
}

void
mat4Ortho(m4 d, cf32 l, cf32 r, cf32 b, cf32 t, cf32 n, cf32 f)
{
    /*	m4 ortho = {
     *		{ 2/(R - L),     0,            0,           0},
     *		{ 0,             2/(T-B),      0,           0},
     *		{ 0,             0,           -2/(F-N),     0},
     *		{ -(R+L)/(R-L), -(T+B)/(T-B), -(F+N)/(F-N), 1}
     *	};
     */

    d[0][0] = 2 / (r - l);
    d[0][1] = 0;
    d[0][2] = 0;
    d[0][3] = 0;

    d[1][0] = 0;
    d[1][1] = 2 / (t - b);
    d[1][2] = 0;
    d[1][3] = 0;

    d[2][0] = 0;
    d[2][1] = 0;
    d[2][2] = -2 / (f - n);
    d[2][3] = 0;

    d[3][0] = -(r + l) / (r - l);
    d[3][1] = -(t + b) / (t - b);
    d[3][2] = -(f + n) / (f - n);
    d[3][3] = 1;
}

static void
mat4LookAtInternal(m4 d, cv3 R, cv3 U, cv3 D, cv3 P)
{
    /* NOTE: it has to be initialized anyway, otherwise weird bugs occur. */
    MAT4_IDENT(d);

    m4 m0 {
        {R[0], U[0], D[0], 0},
        {R[1], U[1], D[1], 0},
        {R[2], U[2], D[2], 0},
        {0,    0,    0,    1}
    };
    mat4Trans(m0, v3 {-P[0], -P[1], -P[2]});
    mat4Mul(d, m0);
}

void
mat4LookAt(m4 d, cv3 eyeV, cv3 centerV, cv3 upV)
{
    v3 camDir;
    vec3Sub3(camDir, eyeV, centerV);
    vec3Norm(camDir);

    v3 camRight;
    VEC3_CROSS3(camRight, upV, camDir);
    vec3Norm(camRight);

    v3 camUp;
    VEC3_CROSS3(camUp, camDir, camRight);

    mat4LookAtInternal(d, camRight, camUp, camDir, eyeV);
}
