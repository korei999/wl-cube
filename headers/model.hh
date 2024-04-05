#pragma once
#include "gmath.hh"
#include "shader.hh"

#include <GLES3/gl32.h>
#include <string_view>
#include <vector>

using VertexPos = std::array<int, 3>;

struct Ubo
{
    GLuint id;
    size_t size;
    GLuint point;

    Ubo() = default;
    Ubo(size_t _size, GLint drawMode);
    ~Ubo();

    void createBuffer(size_t _size, GLint drawMode);
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
    Model(const Model& other) = delete;
    Model(Model&& other);
    Model(std::string_view path);
    ~Model();

    Model& operator=(const Model& other) = delete;
    Model& operator=(Model&& other);

    void loadOBJ(std::string_view path, GLint drawMode = GL_STATIC_DRAW);
    void draw();
    void draw(size_t i);
    void draw(const Mesh& mesh);
    void drawInstanced(GLsizei count);
    void drawInstanced(size_t i, GLsizei count);
    void drawInstanced(const Mesh& mesh, GLsizei count);

private:
    void parseOBJ(std::string_view path, GLint drawMode);
    /* copy buffers to the gpu */
    void setBuffers(std::vector<Vertex>& vs, std::vector<GLuint>& els, Mesh& mesh, GLint drawMode);
};

inline u64
hashFaceVertex(const VertexPos& p)
{
    auto cantorPair = [](u64 a, u64 b) -> u64
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

Model getQuad(GLint drawMode = GL_STATIC_DRAW);
Model getPlane(GLint drawMode = GL_STATIC_DRAW);
void drawQuad(const Model& q);
void drawPlane(const Model& q);
