#pragma once

#include <functional>

#include "gltf/gltf.hh"
#include "gmath.hh"
#include "shader.hh"
#include "texture.hh"
#include "app.hh"

enum class DRAW : int
{
    NONE = 0,
    TEX = 1, /* bind texture for each drawcall */
    APPLY_TM = 1 << 1 /* apply transformation matrix */
};

static inline bool
operator&(enum DRAW l, enum DRAW r)
{
    return static_cast<int>(l) & static_cast<int>(r);
}

static inline enum DRAW
operator|(enum DRAW l, enum DRAW r)
{
    return static_cast<enum DRAW>(static_cast<int>(l) | static_cast<int>(r));
}

static inline enum DRAW
operator^(enum DRAW l, enum DRAW r)
{
    return static_cast<enum DRAW>(static_cast<int>(l) ^ static_cast<int>(r));
}

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
    v3 tan;
    v3 bitan;
};

struct Materials
{
    Texture diffuse;
    Texture normal;
};

struct MeshData
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint eboSize;

    Materials materials;

    std::string name;
};

struct Mesh2
{
    MeshData meshData;

    enum gltf::COMPONENT_TYPE indType;
    enum gltf::PRIMITIVES mode;
    size_t triangleCount;
    v3 vScale {1, 1, 1};
};

struct Model
{
    std::vector<std::vector<MeshData>> objects;
    std::string_view savedPath;

    std::vector<Mesh2> aM2s;
    gltf::Asset asset;

    Model() = default;
    Model(const Model& other) = delete;
    Model(Model&& other);
    Model(std::string_view path, GLint drawMode, GLint texMode, App* c);
    ~Model();

    Model& operator=(const Model& other) = delete;
    Model& operator=(Model&& other);

    void loadOBJ(std::string_view path, GLint drawMode, GLint texMode, App* c);
    void loadGLTF(std::string_view path, GLint drawMode, GLint texMode, App* c);
    void draw();
    void drawGLTF(enum DRAW flags, Shader* sh, std::string_view svUniform, const m4& tm);
    void drawInstanced(GLsizei count);
    /* bind texture for each drawcall */
    void drawTex(GLint primitives = GL_TRIANGLES);

private:
    void parseOBJ(std::string_view path, GLint drawMode, GLint texMode, App* c);
};

inline u64
hashFaceVertex(const FacePositions& p)
{
    auto cantorPair = [](u64 a, u64 b) -> u64 {
        return ((a + b + 1) * ((a + b) / 2) + b);
    };

    return cantorPair(cantorPair(p.x, p.y), p.z);
}

namespace std
{
    template<>
    struct hash<FacePositions>
    {
        u64
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
