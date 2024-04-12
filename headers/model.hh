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

struct Materials
{
    /* TODO: add other materials */
    Texture diffuse;
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

struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint eboSize;

    Texture diffuse;

    std::string name;
};

enum class BindTextures : s8
{
    bind,
    noBind
};

struct Model
{
    std::vector<Mesh> meshes;
    std::string_view savedPath;

    Model() = default;
    Model(const Model& other) = delete;
    Model(Model&& other);
    Model(std::string_view path);
    ~Model();

    Model& operator=(const Model& other) = delete;
    Model& operator=(Model&& other);

    void loadOBJ(std::string_view path, GLint drawMode = GL_STATIC_DRAW, WlClient* c = nullptr);
    void draw();
    void draw(BindTextures draw);
    void draw(size_t i);
    void draw(const Mesh& mesh);
    void drawInstanced(GLsizei count);
    void drawInstanced(size_t i, GLsizei count);
    void drawInstanced(const Mesh& mesh, GLsizei count);

private:
    void parseOBJ(std::string_view path, GLint drawMode, WlClient* c);
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
