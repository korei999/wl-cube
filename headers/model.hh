#pragma once
#include "gmath.hh"
#include "shader.hh"
#include "texture.hh"
#include "wayland.hh"

struct FacePositions
{
    int x, y, z;

    bool
    operator==(const FacePositions& other) const
    {
        return this->x == other.x &&
               this->y == other.y &&
               this->z == other.z; 
    }
};

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

struct Materials
{
    Texture diffuse;
    Texture normal;
};

struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint eboSize;

    Materials materials;

    std::string name;
};

struct Model
{
    std::vector<std::vector<Mesh>> objects;
    std::string_view savedPath;

    Model() = default;
    Model(const Model& other) = delete;
    Model(Model&& other);
    Model(std::string_view path, GLint drawMode, GLint texMode, WlClient* c);
    ~Model();

    Model& operator=(const Model& other) = delete;
    Model& operator=(Model&& other);

    void loadOBJ(std::string_view path, GLint drawMode, GLint texMode, WlClient* c);
    void draw();
    void drawInstanced(GLsizei count);
    /* bind texture for each drawcall */
    void drawTex(GLint primitives = GL_TRIANGLES);

private:
    void parseOBJ(std::string_view path, GLint drawMode, GLint texMode, WlClient* c);
    /* copy buffers to the gpu */
    void setBuffers(std::vector<Vertex>& vs, std::vector<GLuint>& els, Mesh& mesh, GLint drawMode, WlClient* c);
};

inline u64
hashFaceVertex(const FacePositions& p)
{
    auto cantorPair = [](u64 a, u64 b) -> u64
    {
        return ((a + b + 1) * ((a + b) / 2) + b);
    };

    return cantorPair(cantorPair(p.x, p.y), p.z);
}

namespace std
{
    template <>
    struct hash<FacePositions>
    {
        size_t
        operator()(const FacePositions& p) const
        {
            return hashFaceVertex(p);
        }
    };
}

Model getQuad(GLint drawMode = GL_STATIC_DRAW);
Model getPlane(GLint drawMode = GL_STATIC_DRAW);
Model getCube(GLint drawMode = GL_STATIC_DRAW);
void drawQuad(const Model& q);
void drawPlane(const Model& q);
void drawCube(const Model& q);
