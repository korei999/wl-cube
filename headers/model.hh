#pragma once
#include "gmath.hh"
#include "shader.hh"

#include <GLES3/gl3.h>
#include <string_view>
#include <vector>

using VertexPos = std::array<int, 3>;

struct Ubo
{
    GLuint id;
    size_t size;
    GLuint point;

    Ubo() = default;
    Ubo(size_t _size);
    ~Ubo();

    void createBuffer(size_t size);
    void bindBlock(Shader* sh, std::string_view block, GLuint _point);
    void bufferData(void* data, size_t offset, size_t _size);
};

struct Vertex
{
    v3 pos;
    v2 tex;
    v3 norm;
};

struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint eboSize;
};

struct Model
{
    std::vector<Mesh> meshes;

    Model() = default;
    Model(std::string_view path);
    ~Model();

    void loadOBJ(std::string_view path);
    void draw();
    void draw(const Mesh& mesh);
    void draw(size_t i);

private:
    void parseOBJ(std::string_view path);
    /* copy buffers to the gpu */
    void setBuffers(std::vector<Vertex>& vs, std::vector<GLuint>& els, Mesh& mesh);
};

inline u64
hashFaceVertex(const VertexPos& p)
{
    auto cantorPair = [](int a, int b) -> u64
    {
        return ((a + b + 1) * ((a + b) / 2) + b);
    };

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
