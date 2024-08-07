#ifdef LOGS
    #include "utils.hh"
#endif

#include "gmath.hh"

static m4 m4LookAtInternal(const v3& R, const v3& U, const v3& D, const v3& P);

v2::v2(const v3& v)
    : x(v.x), y(v.y) {}

v3::v3(const v2& v)
    : x(v.x), y(v.y) {}

v3::v3(const v4& v)
    : x(v.x), y(v.y), z(v.z) {}

v4::v4(const qt& q)
    : x(q.x), y(q.y), z(q.z), w(q.s) {}

f32
v3Length(const v3& v)
{
    f32 res = 0;
    res += sq(v.x);
    res += sq(v.y);
    res += sq(v.z);

    return std::sqrt(res);
}

f32
v4Length(const v4& v)
{
    f32 res = 0;
    res += sq(v.x);
    res += sq(v.y);
    res += sq(v.z);
    res += sq(v.w);

    return std::sqrt(res);
}

v3
v3Norm(const v3& v)
{
    f32 len = v3Length(v);
    return v3 {v.x / len, v.y / len, v.z / len};
}

v4
v4Norm(const v4& v)
{
    f32 len = v4Length(v);
    return {v.x / len, v.y / len, v.z / len, v.w / len};
}

v3
v3Norm(const v3& v, const f32 length)
{
    return v3 {v.x / length, v.y / length, v.z / length};
}

v3
v3Cross(const v3& l, const v3& r)
{
    return v3 {
        (l.y * r.z) - (r.y * l.z),
        (l.z * r.x) - (r.z * l.x),
        (l.x * r.y) - (r.x * l.y)
    };
}

v3
operator-(const v3& l, const v3& r)
{
    return v3 {
        l.x - r.x,
        l.y - r.y,
        l.z - r.z
    };
}

v2
operator-(const v2& l, const v2& r)
{
    return v2 {
        l.x - r.x,
        l.y - r.y
    };
}

v3
operator+(const v3& l, const v3& r)
{
    return v3 {
        l.x + r.x,
        l.y + r.y,
        l.z + r.z
    };
}

v3&
v3::operator+=(const v3& other)
{
    *this = *this + other;
    return *this;
}

v3&
v3::operator-=(const v3& other)
{
    *this = *this - other;
    return *this;
}

v3
operator*(const v3& v, const f32 s)
{
    return v3 {
        v.x * s,
        v.y * s,
        v.z * s
    };
}

v3&
v3::operator*=(const f32 s)
{
    *this = *this * s;
    return *this;
}

f32
v3Rad(const v3& l, const v3& r)
{
    return std::acos(v3Dot(l, r) / (v3Length(l) * v3Length(r)));
}

f32
v3Dist(const v3& l, const v3& r)
{
    return std::sqrt(sq(r.x - l.x) + sq(r.y - l.y) + sq(r.z - l.z));
}

f32
v3Dot(const v3& l, const v3& r)
{
    return (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
}

f32
v4Dot(const v4& l, const v4& r)
{
    return (l.x * r.x) + (l.y * r.y) + (l.z * r.z) + (l.w * r.w);
}

m4&
m4::operator*=(const m4& other)
{
    *this = *this * other;
    return *this;
}

m4
operator*(const m4& l, const m4& r)
{
    m4 res;
    for (int i = 0; i < 4; i++)
    {
        res.e[i][0] = (l.e[0][0]*r.e[i][0]) + (l.e[1][0]*r.e[i][1]) + (l.e[2][0]*r.e[i][2]) + (l.e[3][0]*r.e[i][3]); 
        res.e[i][1] = (l.e[0][1]*r.e[i][0]) + (l.e[1][1]*r.e[i][1]) + (l.e[2][1]*r.e[i][2]) + (l.e[3][1]*r.e[i][3]); 
        res.e[i][2] = (l.e[0][2]*r.e[i][0]) + (l.e[1][2]*r.e[i][1]) + (l.e[2][2]*r.e[i][2]) + (l.e[3][2]*r.e[i][3]); 
        res.e[i][3] = (l.e[0][3]*r.e[i][0]) + (l.e[1][3]*r.e[i][1]) + (l.e[2][3]*r.e[i][2]) + (l.e[3][3]*r.e[i][3]); 
    }

    return res;
}

m4
m4Rot(const m4& m, const f32 th, const v3& ax)
{
    const f32 c = std::cos(th);
    const f32 s = std::sin(th);

    const f32 x = ax.x;
    const f32 y = ax.y;
    const f32 z = ax.z;

    m4 r {.e {
        {((1 - c)*sq(x)) + c, ((1 - c)*x*y) - s*z, ((1 - c)*x*z) + s*y, 0},
        {((1 - c)*x*y) + s*z, ((1 - c)*sq(y)) + c, ((1 - c)*y*z) - s*x, 0},
        {((1 - c)*x*z) - s*y, ((1 - c)*y*z) + s*x, ((1 - c)*sq(z)) + c, 0},
        {0,                   0,                   0,                   1}
    }};

    return m * r;
}

m4
m4RotX(const m4& m, const f32 angle)
{
    m4 axisX {.e {
        {1, 0,                0,               0},
        {0, std::cos(angle),  std::sin(angle), 0},
        {0, -std::sin(angle), std::cos(angle), 0},
        {0, 0,                0,               1}
    }};

    return m * axisX;
}

m4
m4RotY(const m4& m, const f32 angle)
{
    m4 axisY {.e {
        {std::cos(angle), 0, -std::sin(angle), 0},
        {0,               1, 0,                0},
        {std::sin(angle), 0, std::cos(angle),  0},
        {0,               0, 0,                1}
    }};

    return m * axisY;
}

m4
m4RotZ(const m4& m, const f32 angle)
{
    m4 axisZ {.e {
        {std::cos(angle),  std::sin(angle), 0, 0},
        {-std::sin(angle), std::cos(angle), 0, 0},
        {0,                0,               1, 0},
        {0,                0,               0, 1}
    }};

    return m * axisZ;
}

m4
m4Scale(const m4& m, const f32 s)
{
    m4 sm {.e {
        {s, 0, 0, 0},
        {0, s, 0, 0},
        {0, 0, s, 0},
        {0, 0, 0, 1}
    }};

    return m * sm;
}

m4
m4Scale(const m4& m, const v3& s)
{
    m4 sm {.e {
        {s.x, 0,   0,   0},
        {0,   s.y, 0,   0},
        {0,   0,   s.z, 0},
        {0,   0,   0,   1}
    }};

    return m * sm;
}

m4
m4Translate(const m4& m, const v3& tv)
{
    m4 tm {.e {
        {1,    0,    0,    0},
        {0,    1,    0,    0},
        {0,    0,    1,    0},
        {tv.x, tv.y, tv.z, 1}
    }};

    return m * tm;
}

m4
m4Pers(const f32 fov, const f32 asp, const f32 n, const f32 f)
{
    /* b(back), l(left) are not needed if viewing volume is symmetric */
    f32 t = n * std::tan(fov / 2);
    f32 r = t * asp;

    return m4 {.e {
        {n / r, 0,     0,                  0},
        {0,     n / t, 0,                  0},
        {0,     0,    -(f + n) / (f - n), -1},
        {0,     0,    -(2*f*n) / (f - n),  0}
    }};
}

m4
m4Ortho(const f32 l, const f32 r, const f32 b, const f32 t, const f32 n, const f32 f)
{
    return m4 {.e {
        {2/(r-l),       0,            0,           0},
        {0,             2/(t-b),      0,           0},
        {0,             0,           -2/(f-n),     0},
        {-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1}
    }};
}

static m4
m4LookAtInternal(const v3& R, const v3& U, const v3& D, const v3& P)
{
    m4 m0 {.e {
        {R.x,  U.x,  D.x,  0},
        {R.y,  U.y,  D.y,  0},
        {R.z,  U.z,  D.z,  0},
        {0,    0,    0,    1}
    }};

    return (m4Translate(m0, {-P.x, -P.y, -P.z}));
}

m4
m4LookAt(const v3& eyeV, const v3& centerV, const v3& upV)
{
    v3 camDir = v3Norm(eyeV - centerV);
    v3 camRight = v3Norm(v3Cross(upV, camDir));
    v3 camUp = v3Cross(camDir, camRight);

    return m4LookAtInternal(camRight, camUp, camDir, eyeV);
}

m4
m4Transpose(const m4& m)
{
    auto e = m.e;
    return {.e {
        {e[0][0], e[1][0], e[2][0], e[3][0]},
        {e[0][1], e[1][1], e[2][1], e[3][1]},
        {e[0][2], e[1][2], e[2][2], e[3][2]},
        {e[0][3], e[1][3], e[2][3], e[3][3]}
    }};
}

m3
m3Transpose(const m3& m)
{
    auto e = m.e;
    return {
        e[0][0], e[1][0], e[2][0],
        e[0][1], e[1][1], e[2][1],
        e[0][2], e[1][2], e[2][2]
    };
}

#ifdef LOGS
std::string
m4ToString(const m4& m, std::string_view prefix)
{
    auto e = m.e;
    return FMT("{}:\n\t{:.3f} {:.3f} {:.3f} {:.3f}\n\t{:.3f} {:.3f} {:.3f} {:.3f}\n\t{:.3f} {:.3f} {:.3f} {:.3f}\n\t{:.3f} {:.3f} {:.3f} {:.3f}\n",
        prefix,
        e[0][0], e[0][1], e[0][2], e[0][3],
        e[1][0], e[1][1], e[1][2], e[1][3],
        e[2][0], e[2][1], e[2][2], e[2][3],
        e[3][0], e[3][1], e[3][2], e[3][3]);
}

std::string
m3ToString(const m3& m, std::string_view prefix)
{
    auto e = m.e;
    return FMT("{}:\n\t{:.3f} {:.3f} {:.3f}\n\t{:.3f} {:.3f} {:.3f}\n\t{:.3f} {:.3f} {:.3f}\n\t{:.3f} {:.3f} {:.3f}\n",
        prefix,
        e[0][0], e[0][1], e[0][2],
        e[1][0], e[1][1], e[1][2],
        e[2][0], e[2][1], e[2][2],
        e[3][0], e[3][1], e[3][2]);
}

std::string
v4ToString(const v4& v, std::string_view prefix)
{
    return FMT("{}:\n\t{:.3f} {:.3f} {:.3f} {:.3f}\n", prefix, v.x, v.y, v.z, v.w);
}
#endif

m3
m3Inverse(const m3& m)
{
    auto e = m.e;
    f32 det = e[0][0] * (e[1][1] * e[2][2] - e[2][1] * e[1][2]) -
              e[0][1] * (e[1][0] * e[2][2] - e[1][2] * e[2][0]) +
              e[0][2] * (e[1][0] * e[2][1] - e[1][1] * e[2][0]);
    f32 invdet = 1.0f / det;

    return {
        (e[1][1] * e[2][2] - e[2][1] * e[1][2]) * invdet,
        (e[0][2] * e[2][1] - e[0][1] * e[2][2]) * invdet,
        (e[0][1] * e[1][2] - e[0][2] * e[1][1]) * invdet,

        (e[1][2] * e[2][0] - e[1][0] * e[2][2]) * invdet,
        (e[0][0] * e[2][2] - e[0][2] * e[2][0]) * invdet,
        (e[1][0] * e[0][2] - e[0][0] * e[1][2]) * invdet,

        (e[1][0] * e[2][1] - e[2][0] * e[1][1]) * invdet,
        (e[2][0] * e[0][1] - e[0][0] * e[2][1]) * invdet,
        (e[0][0] * e[1][1] - e[1][0] * e[0][1]) * invdet
    };
}

m3 
m3Normal(const m3& m)
{
    return m3Transpose(m3Inverse(m));
}

v3
v3Color(const u32 hex)
{
    v3 t = COLOR3(hex);
    return t;
}

v4
v4Color(const u32 hex)
{
    v4 t = COLOR4(hex);
    return t;
}

qt
qtAxisAngle(const v3& axis, f32 th)
{
    f32 sinTh = static_cast<f32>(std::sin(th / 2));

    return {
        axis.x * sinTh,
        axis.y * sinTh,
        axis.z * sinTh,
        static_cast<f32>(cos(th / 2))
    };
}

m4
qtRot(const qt& q)
{
    auto& x = q.x;
    auto& y = q.y;
    auto& z = q.z;
    auto& s = q.s;

    return {.e {
        {1 - 2*y*y - 2*z*z, 2*x*y - 2*s*z,     2*x*z + 2*s*y,     0},
        {2*x*y + 2*s*z,     1 - 2*x*x - 2*z*z, 2*y*z - 2*s*x,     0},
        {2*x*z - 2*s*y,     2*y*z + 2*s*x,     1 - 2*x*x - 2*y*y, 0},
        {0,                 0,                 0,                 1}
    }};
}

qt
qtConj(const qt& q)
{
    return {-q.x, -q.y, -q.z, q.s};
}

qt
operator*(const qt& l, const qt& r)
{
    return {
        l.s*r.x + l.x*r.s + l.y*r.z - l.z*r.y,
        l.s*r.y - l.x*r.z + l.y*r.s + l.z*r.x,
        l.s*r.z + l.x*r.y - l.y*r.x + l.z*r.s,
        l.s*r.s - l.x*r.x - l.y*r.y - l.z*r.z,
    };
}

qt
operator*=(qt& l, const qt& r)
{
    return l = l * r;
}
