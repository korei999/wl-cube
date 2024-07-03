#pragma once
#include <string_view>

#include "../json/parser.hh"
#include "headers/gmath.hh"
#include "headers/utils.hh"

namespace gltf
{

enum class HASH_CODES : u64
{
    scene = hashFNV("scene"),
    scenes = hashFNV("scenes"),
    nodes = hashFNV("nodes"),
    meshes = hashFNV("meshes"),
    cameras = hashFNV("cameras"),
    buffers = hashFNV("buffers"),
    bufferViews = hashFNV("bufferViews"),
    accessors = hashFNV("accessors"),
    materials = hashFNV("materials"),
    textures = hashFNV("textures"),
    images = hashFNV("images"),
    samplers = hashFNV("samplers"),
    skins = hashFNV("skins"),
    animations = hashFNV("animations"),
    SCALAR = hashFNV("SCALAR"),
    VEC2 = hashFNV("VEC2"),
    VEC3 = hashFNV("VEC3"),
    VEC4 = hashFNV("VEC4"),
    MAT3 = hashFNV("MAT3"),
    MAT4 = hashFNV("MAT4")
};

enum class COMPONENT_TYPE
{
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126
};

constexpr std::string_view
getComponentTypeString(enum COMPONENT_TYPE t)
{
    switch (t)
    {
        default:
        case COMPONENT_TYPE::BYTE:
            return "BYTE";
        case COMPONENT_TYPE::UNSIGNED_BYTE:
            return "UNSIGNED_BYTE";
        case COMPONENT_TYPE::SHORT:
            return "SHORT";
        case COMPONENT_TYPE::UNSIGNED_SHORT:
            return "UNSIGNED_SHORT";
        case COMPONENT_TYPE::UNSIGNED_INT:
            return "UNSIGNED_INT";
        case COMPONENT_TYPE::FLOAT:
            return "FLOAT";
    }
}

enum class TARGET
{
    NONE = 0,
    ARRAY_BUFFER = 34962,
    ELEMENT_ARRAY_BUFFER = 34963
};

constexpr std::string_view
getTargetString(enum TARGET t)
{
    switch (t)
    {
        default:
        case TARGET::NONE:
            return "NONE";
        case TARGET::ARRAY_BUFFER:
            return "ARRAY_BUFFER";
        case TARGET::ELEMENT_ARRAY_BUFFER:
            return "ELEMENT_ARRAY_BUFFER";
    }
}

struct Nodes
{
    json::Object* scene;
    json::Object* scenes;
    json::Object* nodes;
    json::Object* meshes;
    json::Object* cameras;
    json::Object* buffers;
    json::Object* bufferViews;
    json::Object* accessors;
    json::Object* materials;
    json::Object* textures;
    json::Object* images;
    json::Object* samplers;
    json::Object* skins;
    json::Object* animations;
};

struct Scene
{
    size_t nodeIdx;
};

struct Buffer
{
    size_t byteLength;
    std::string_view uri;
    std::vector<char> aBin;
};

enum class ACCESSOR_TYPE
{
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    /*MAT2, Unused*/
    MAT3,
    MAT4
};

union Type
{
    size_t SCALAR;
    v2 VEC2;
    v3 VEC3;
    v4 VEC4;
    /*m2 MAT2; Unused */
    m3 MAT3;
    m4 MAT4;
};

struct Accessor
{
    size_t bufferView;
    size_t byteOffset;
    enum COMPONENT_TYPE componentType; /* REQUIRED */
    size_t count; /* REQUIRED */
    union Type max;
    union Type min;
    enum ACCESSOR_TYPE type; /* REQUIRED */
};

struct Node
{
    size_t camera;
    std::vector<size_t> children;
    m4 matrix = m4Iden();
    size_t mesh;
    /*v4 rotation; not implemented */
    v3 scale = {1, 1, 1};
    v3 translation;
};

struct CameraPersp
{
    f64 aspectRatio;
    f64 yfov;
    f64 zfar;
    f64 znear;
};

struct CameraOrtho
{
    //
};

struct Camera
{
    union {
        CameraPersp perspective;
        CameraOrtho orthographic;
    } proj;
    enum {
        perspective,
        orthographic
    } type;
};

struct BufferView
{
    size_t buffer;
    size_t byteOffset;
    size_t byteLength;
    size_t byteStride;
    enum TARGET target;
};

struct Image
{
    std::string_view svUri;
};

struct Primitive
{
};

struct Mesh
{
    std::vector<Primitive> aPrimitives; /* REQUIRED */
};

struct Asset
{
    json::Parser p;
    std::string_view svGenerator;
    std::string_view svVersion;
    Nodes nodes {};
    size_t defaultSceneIdx;
    std::vector<Scene> aScenes;
    std::vector<Buffer> aBuffers;
    std::vector<BufferView> aBufferViews;
    std::vector<Image> aImages;
    std::vector<Accessor> aAccessors;

    Asset(std::string_view path);
};

} /* namespace gltf */
