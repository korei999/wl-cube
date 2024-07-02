#pragma once
#include <string_view>

#include "../json/parser.hh"

namespace gltf
{

struct Nodes
{
    json::Node* nScene;
    json::Node* nScenes;
    json::Node* nNodes;
    json::Node* nMeshes;
    json::Node* nBuffers;
    json::Node* nBufferViews;
    json::Node* nAccessors;
    json::Node* nMaterials;
    json::Node* nTextures;
    json::Node* nImages;
    json::Node* nSamplers;
    json::Node* nSkins;
    json::Node* nAnimations;
};

struct Scene
{
    size_t nodeIdx;
};

struct Buffer
{
    size_t byteLength;
    std::string_view svUri;
    std::vector<char> aBin;
};

struct Image
{
    std::string_view svUri;
};

struct Model
{
    json::Parser p;
    Nodes nodes {};
    size_t defaultSceneIdx;
    std::vector<Scene> aScenes;
    std::vector<Buffer> aBuffers;
    std::vector<Image> aImages;

    Model(std::string_view path);

    void printJSON();
};

} /* namespace gltf */
