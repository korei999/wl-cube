#pragma once
#include <string_view>

#include "../json/parser.hh"
#include "headers/gmath.hh"

namespace gltf
{

enum class COMPONENT_TYPE
{
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126
};

enum class TARGET
{
    NONE = 0,
    ARRAY_BUFFER = 34962,
    ELEMENT_ARRAY_BUFFER = 34963
};

struct Nodes
{
    json::KeyVal* scene;
    json::KeyVal* scenes;
    json::KeyVal* nodes;
    json::KeyVal* meshes;
    json::KeyVal* cameras;
    json::KeyVal* buffers;
    json::KeyVal* bufferViews;
    json::KeyVal* accessors;
    json::KeyVal* materials;
    json::KeyVal* textures;
    json::KeyVal* images;
    json::KeyVal* samplers;
    json::KeyVal* skins;
    json::KeyVal* animations;
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

union Type
{
    size_t SCALAR;
    v3 VEC3;
    v2 VEC2;
    v4 VEC4;
    /*m2 MAT2; Unused */
    m3 MAT3;
    m4 MAT4;
};

struct Accessor
{
    size_t buffView;
    size_t byteOffset;
    enum COMPONENT_TYPE componentType;
    size_t count;
    Type max;
    Type min;
    std::string_view type;
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

    Asset(std::string_view path);
};

} /* namespace gltf */
