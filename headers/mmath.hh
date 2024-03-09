#pragma once
#include "ultratypes.h"

typedef f32 v2[2];
typedef f32 v3[3];
typedef f32 v4[4];
typedef f32 m4[4][4];

typedef const v2 cv2;
typedef const v3 cv3;
typedef const v4 cv4;
typedef const m4 cm4;

#define PI 3.14159265358979323846

#define SQ(X) (X * X)
#define TO_DEG(X) (X * 180.0 / PI)
#define TO_RAD(X) (X * PI / 180.0)

#define MAT4_MUL3(D, S0, S1)                                                                                           \
    {                                                                                                                  \
        /* multiply S0 and S1 matrices and store in D */                                                               \
        D[0][0] = (S0[0][0] * S1[0][0]) + (S0[1][0] * S1[0][1]) + (S0[2][0] * S1[0][2]) + (S0[3][0] * S1[0][3]);       \
        D[1][0] = (S0[0][0] * S1[1][0]) + (S0[1][0] * S1[1][1]) + (S0[2][0] * S1[1][2]) + (S0[3][0] * S1[1][3]);       \
        D[2][0] = (S0[0][0] * S1[2][0]) + (S0[1][0] * S1[2][1]) + (S0[2][0] * S1[2][2]) + (S0[3][0] * S1[2][3]);       \
        D[3][0] = (S0[0][0] * S1[3][0]) + (S0[1][0] * S1[3][1]) + (S0[2][0] * S1[3][2]) + (S0[3][0] * S1[3][3]);       \
                                                                                                                       \
        D[0][1] = (S0[0][1] * S1[0][0]) + (S0[1][1] * S1[0][1]) + (S0[2][1] * S1[0][2]) + (S0[3][1] * S1[0][3]);       \
        D[1][1] = (S0[0][1] * S1[1][0]) + (S0[1][1] * S1[1][1]) + (S0[2][1] * S1[1][2]) + (S0[3][1] * S1[1][3]);       \
        D[2][1] = (S0[0][1] * S1[2][0]) + (S0[1][1] * S1[2][1]) + (S0[2][1] * S1[2][2]) + (S0[3][1] * S1[2][3]);       \
        D[3][1] = (S0[0][1] * S1[3][0]) + (S0[1][1] * S1[3][1]) + (S0[2][1] * S1[3][2]) + (S0[3][1] * S1[3][3]);       \
                                                                                                                       \
        D[0][2] = (S0[0][2] * S1[0][0]) + (S0[1][2] * S1[0][1]) + (S0[2][2] * S1[0][2]) + (S0[3][2] * S1[0][3]);       \
        D[1][2] = (S0[0][2] * S1[1][0]) + (S0[1][2] * S1[1][1]) + (S0[2][2] * S1[1][2]) + (S0[3][2] * S1[1][3]);       \
        D[2][2] = (S0[0][2] * S1[2][0]) + (S0[1][2] * S1[2][1]) + (S0[2][2] * S1[2][2]) + (S0[3][2] * S1[2][3]);       \
        D[3][2] = (S0[0][2] * S1[3][0]) + (S0[1][2] * S1[3][1]) + (S0[2][2] * S1[3][2]) + (S0[3][2] * S1[3][3]);       \
                                                                                                                       \
        D[0][3] = (S0[0][3] * S1[0][0]) + (S0[1][3] * S1[0][1]) + (S0[2][3] * S1[0][2]) + (S0[3][3] * S1[0][3]);       \
        D[1][3] = (S0[0][3] * S1[1][0]) + (S0[1][3] * S1[1][1]) + (S0[2][3] * S1[1][2]) + (S0[3][3] * S1[1][3]);       \
        D[2][3] = (S0[0][3] * S1[2][0]) + (S0[1][3] * S1[2][1]) + (S0[2][3] * S1[2][2]) + (S0[3][3] * S1[2][3]);       \
        D[3][3] = (S0[0][3] * S1[3][0]) + (S0[1][3] * S1[3][1]) + (S0[2][3] * S1[3][2]) + (S0[3][3] * S1[3][3]);       \
    }

#define MAT4_IDENT(D)                                                                                                  \
    /* store an identity matrix in D */                                                                                \
    (D[0][0] = 1, D[0][1] = 0, D[0][2] = 0, D[0][3] = 0, D[1][0] = 0, D[1][1] = 1, D[1][2] = 0, D[1][3] = 0,           \
     D[2][0] = 0, D[2][1] = 0, D[2][2] = 1, D[2][3] = 0, D[3][0] = 0, D[3][1] = 0, D[3][2] = 0, D[3][3] = 1)

#define IDENT                                                                                                          \
    {                                                                                                                  \
        /* Initialize m4 as identity matrix. Works with '=' operator. */                                               \
        {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0},                                                                      \
        {                                                                                                              \
            0, 0, 0, 1                                                                                                 \
        }                                                                                                              \
    }

#define MAT4_PRINT(X)                                                                                                  \
    {                                                                                                                  \
        for (int i = 0; i < 4; i++)                                                                                    \
        {                                                                                                              \
            for (int j = 0; j < 4; j++)                                                                                \
                std::print(stderr, "{:5.2}, ", X[i][j]);                                                               \
            putc('\n', stderr);                                                                                        \
        }                                                                                                              \
    }

#define VEC3_PRINT(X)                                                                                                  \
    {                                                                                                                  \
        for (int i = 0; i < 3; i++)                                                                                    \
            LOG("{}, ", X[i]);                                                                                         \
        putc('\n', stderr);                                                                                            \
    }

#define VEC3_COPY(D, S)                                                                                                \
    {                                                                                                                  \
        D[0] = S[0];                                                                                                   \
        D[1] = S[1];                                                                                                   \
        D[2] = S[2];                                                                                                   \
    }

#define VEC2_COPY(D, S)                                                                                                \
    {                                                                                                                  \
        D[0] = S[0];                                                                                                   \
        D[1] = S[1];                                                                                                   \
    }
#define MAT4_COPY(D, S)                                                                                                \
    {                                                                                                                  \
        D[0][0] = S[0][0];                                                                                             \
        D[0][1] = S[0][1];                                                                                             \
        D[0][2] = S[0][2];                                                                                             \
        D[0][3] = S[0][3];                                                                                             \
                                                                                                                       \
        D[1][0] = S[1][0];                                                                                             \
        D[1][1] = S[1][1];                                                                                             \
        D[1][2] = S[1][2];                                                                                             \
        D[1][3] = S[1][3];                                                                                             \
                                                                                                                       \
        D[2][0] = S[2][0];                                                                                             \
        D[2][1] = S[2][1];                                                                                             \
        D[2][2] = S[2][2];                                                                                             \
        D[2][3] = S[2][3];                                                                                             \
                                                                                                                       \
        D[3][0] = S[3][0];                                                                                             \
        D[3][1] = S[3][1];                                                                                             \
        D[3][2] = S[3][2];                                                                                             \
        D[3][3] = S[3][3];                                                                                             \
    }

#define VEC3_MAT4_MUL(D, S)                                                                                            \
    {                                                                                                                  \
        /* multiply vector D by matrix S and store in D. */                                                            \
        D[0] = S[0][0] * D[0] + S[1][0] * D[0] + S[2][0] * D[0] + S[3][0] * D[0];                                      \
        D[1] = S[0][1] * D[1] + S[1][1] * D[1] + S[2][1] * D[1] + S[3][1] * D[1];                                      \
        D[2] = S[0][2] * D[2] + S[1][2] * D[2] + S[2][2] * D[2] + S[3][2] * D[2];                                      \
    }

#define VEC3_CROSS3(D, S0, S1)                                                                                         \
    {                                                                                                                  \
        D[0] = S0[1] * S1[2] - S1[1] * S0[2];                                                                          \
        D[1] = S0[2] * S1[0] - S1[2] * S0[0];                                                                          \
        D[2] = S0[0] * S1[1] - S1[0] * S0[1];                                                                          \
    }

/* stores result to DEST  */
void vec3Add(v3 dest, cv3 src);

/* stores result to DEST  */
void vec3Add3(v3 dest, cv3 s0, cv3 s1);

/* stores result to DEST  */
void vec3Sub(v3 dest, cv3 src);

/* stores result of s0 - s1 to DEST  */
void vec3Sub3(v3 dest, cv3 s0, cv3 s1);

/* returns magnitude of the vector */
f32 vec3Length(cv3 src);

/* stores result to DEST */
void vec3Norm(v3 dest);

/* stores normalized S vector to D */
void vec3Norm2(v3 d, cv3 s);

/* stores result to DEST  */
void vec3Scale(v3 dest, cf32 scalar);

/* stores result to DEST  */
void vec3Scale3(v3 dest, cv3 s, cf32 scalar);

/* returns value of dot product */
f32 vec3Dot(cv3 s0, cv3 s1);

/* returns degree(IN RADIANS) between two vectors */
f32 Vec3Rad(cv3 s0, cv3 s1);

/* returns distance between two points in space (vectors) */
f64 Vec3Dist(cv3 s0, cv3 s1);

/* declining sort comparison */
f64 Vec3DistCmpDec(cv3 s0, cv3 s1, cv3 eye);

/* inclining sort comparison */
f64 Vec3DistCmpInc(cv3 s0, cv3 s1, cv3 eye);

/* Sorts array of vectors using FuncCmp function pointer. Made specificaly to sort transperent object positions,
 * so they are drawn in right order.
 * Specificaly made to compare two vector positions against vEye position.
 * Insertion sort should be slightly faster for small lists then any other sorting. */
void vecSortByDistToEye(v3 v[], cv3 vEye, f64 (*FuncCmp)(cv3 s0, cv3 s1, cv3 eye), cu64 size);

/* stores cross product to DEST  */
void Vec3Cross(v3 dest, cv3 src);

/* stores cross product of s0 and s1 to DEST */
void Vec3Cross3(v3 dest, cv3 s0, cv3 s1);

/* stores multiplication to DEST  */
void mat4Add(m4 dest, cm4 src);

/* stores subtraction to DEST  */
void mat4Sub(m4 dest, cm4 src);

/* stores multiplication to DEST. Uniform scaling  */
void mat4Scale(m4 dest, cf32 scalar);

/* stores multiplication to DEST. Non-uniform scaling with vector */
void mat4ScaleV(m4 dest, cv3 vScalar);

/* stores translation to DEST  */
void mat4Trans(m4 dest, cv3 src);

/* stores to DEST  */
void mat4Mul(m4 dest, cm4 src);

/* stores to DEST.
 * Do not call like Mat4Mul3(oneMatrix, oneMatix, anotherMatix).
 * Values depend on each other, we must not modify matrix during calculations.
 * Look at matrix multiplication formula. */
void mat4Mul3(m4 dest, cm4 s0, cm4 s1);

/* stores to DEST  */
void mat4Rotx(m4 dest, cf32 angle);

/* stores to DEST  */
void mat4Roty(m4 dest, cf32 angle);

/* stores to DEST  */
void mat4Rotz(m4 dest, cf32 angle);

/* stores to DEST. Rotates around arbitrary axis.
 * Call Vec3Norm(axis) before this function.
 * Axis vector has to be normalized in order to not distort object's shape. */
void mat4Rot(m4 dest, cf32 angle, cv3 axis);

/* sets DEST to perspective viewing transformation matrix. */
void mat4Pers(m4 dest, cf32 angle, cf32 aspect, cf32 near, cf32 far);

/* sets DEST to orthographic viewing transformation matrix. */
void mat4Ortho(m4 dest, cf32 left, cf32 right, cf32 bottom, cf32 top, cf32 near, cf32 far);

/*
 *  This function computes a matrix and multiplies it with whatever is in dest.
 *  eyeV is a XYZ position. This is where you are (eye/camera).
 *  centerV is the XYZ position where you want to look at.
 *  upV is a XYZ normalized vector ({ 0.0, 1.0, 0.0 }).
 *  There is also internal Mat4LookAtInternal function which is called here. */
void mat4LookAt(m4 dest, cv3 eyeV, cv3 centerV, cv3 upV);
