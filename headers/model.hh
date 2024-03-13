#pragma once
#include "gmath.hh"

#include <GLES3/gl3.h>
#include <string_view>
#include <vector>

using VertexPos = std::array<int, 3>;

struct Vertex
{
    v3 pos;
    v2 tex;
    v3 norm;
};

struct Mesh
{
    std::vector<Vertex> vs;
    std::vector<GLuint> indices;
};

struct Model
{
    Mesh m;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    std::vector<Mesh> meshes;

    Model() = default;
    Model(std::string_view path);

    void loadOBJ(std::string_view path);
    void drawMesh();

private:
    Mesh parseOBJ(std::string_view path);
    void setBuffers();
};

inline u64
hashFaceVertex(const VertexPos& p)
{
    auto cantorPair = [](int a, int b) -> u64
    { return ((a + b + 1) * ((a + b) / 2) + b); };

    return cantorPair(cantorPair(p[0], p[1]), p[2]);
}

namespace std
{
    template <>
    struct hash<VertexPos>
    {
        size_t
        operator()(const VertexPos& p) const
        {
            return hashFaceVertex(p);
        }
    };
}
