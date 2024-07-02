#include "gltf.hh"

#include "../headers/utils.hh"

namespace gltf
{

enum class GLTFHash : u64
{
    scene = hashFNV("scene"),
    scenes = hashFNV("scenes"),
    nodes = hashFNV("nodes"),
    meshes = hashFNV("meshes"),
    buffers = hashFNV("buffers"),
    bufferViews = hashFNV("bufferViews"),
    accessors = hashFNV("accessors"),
    materials = hashFNV("materials"),
    textures = hashFNV("textures"),
    images = hashFNV("images"),
    samplers = hashFNV("samplers"),
    skins = hashFNV("skins"),
    animations = hashFNV("animations")
};

Asset::Asset(std::string_view path)
    : p(path)
{
    p.parse();

    /* collect all the top level objects */
    /*for (auto& node : getNodes(this->p.m_upHead.get()))*/
    for (auto& node : json::getObject(this->p.m_upHead->tagVal.val))
    {
        switch (hashFNV(node.svKey))
        {
            default:
                break;
            case (u64)GLTFHash::scene:
                nodes.nScene = &node;
                break;
            case (u64)GLTFHash::scenes:
                nodes.nScenes = &node;
                break;
            case (u64)GLTFHash::nodes:
                nodes.nNodes = &node;
                break;
            case (u64)GLTFHash::meshes:
                nodes.nMeshes = &node;
                break;
            case (u64)GLTFHash::buffers:
                nodes.nBuffers = &node;
                break;
            case (u64)GLTFHash::bufferViews:
                nodes.nBufferViews = &node;
                break;
            case (u64)GLTFHash::accessors:
                nodes.nAccessors = &node;
                break;
            case (u64)GLTFHash::materials:
                nodes.nMaterials = &node;
                break;
            case (u64)GLTFHash::textures:
                nodes.nTextures = &node;
                break;
            case (u64)GLTFHash::images:
                nodes.nImages = &node;
                break;
            case (u64)GLTFHash::samplers:
                nodes.nSamplers = &node;
                break;
            case (u64)GLTFHash::skins:
                nodes.nSkins = &node;
                break;
            case (u64)GLTFHash::animations:
                nodes.nAnimations = &node;
                break;
        }
    }

#ifdef GLTF
    LOG(OK, "GLTF: '{}'\n", path);
    auto check = [](std::string_view sv, json::Node* p) -> void {
        CERR("\t{}: '{}'\n", sv, p ? p->svKey : "(null)");
    };

    check("scene", this->nodes.nScene);
    check("scenes", this->nodes.nScenes);
    check("nodes", this->nodes.nNodes);
    check("meshes", this->nodes.nMeshes);
    check("buffers", this->nodes.nBuffers);
    check("bufferViews", this->nodes.nBufferViews);
    check("accessors", this->nodes.nAccessors);
    check("materials", this->nodes.nMaterials);
    check("textures", this->nodes.nTextures);
    check("images", this->nodes.nImages);
    check("samplers", this->nodes.nSamplers);
    check("skins", this->nodes.nSkins);
    check("animations", this->nodes.nAnimations);
#endif

    this->defaultSceneIdx = json::getInteger(this->nodes.nScene->tagVal.val);

#ifdef GLTF
    LOG(OK, "defaultSceneIdx: {}\n", this->defaultSceneIdx);
#endif

#ifdef GLTF
        LOG(OK, "processing '{}'...\n", this->nodes.nScenes->svKey);
#endif
    {
        /* TODO: push each index (not used much) */
        auto scenes = this->nodes.nScenes;
        auto& arr = json::getArray(scenes->tagVal.val);
        auto& obj = json::getObject(arr.front().val);
        auto& n0 = json::getObject(obj.front().tagVal.val);
        auto& arrN0 = json::getArray(n0.front().tagVal.val);
        auto number = json::getInteger(arrN0.front().val);
        this->aScenes.push_back({(size_t)number});
    }
#ifdef GLTF
        LOG(OK, "first scene idx: '{}'\n", this->aScenes.front().nodeIdx);
#endif
}

void
Asset::printJSON()
{
    p.print();
}

} /* namespace gltf */
