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
    cameras = hashFNV("cameras"),
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
    for (auto& node : json::getObject(*this->p.m_upHead))
    {
        switch (hashFNV(node.svKey))
        {
            default:
                break;
            case (u64)GLTFHash::scene:
                nodes.scene = &node;
                break;
            case (u64)GLTFHash::scenes:
                nodes.scenes = &node;
                break;
            case (u64)GLTFHash::nodes:
                nodes.nodes = &node;
                break;
            case (u64)GLTFHash::meshes:
                nodes.meshes = &node;
                break;
            case (u64)GLTFHash::cameras:
                nodes.cameras = &node;
                break;
            case (u64)GLTFHash::buffers:
                nodes.buffers = &node;
                break;
            case (u64)GLTFHash::bufferViews:
                nodes.bufferViews = &node;
                break;
            case (u64)GLTFHash::accessors:
                nodes.accessors = &node;
                break;
            case (u64)GLTFHash::materials:
                nodes.materials = &node;
                break;
            case (u64)GLTFHash::textures:
                nodes.textures = &node;
                break;
            case (u64)GLTFHash::images:
                nodes.images = &node;
                break;
            case (u64)GLTFHash::samplers:
                nodes.samplers = &node;
                break;
            case (u64)GLTFHash::skins:
                nodes.skins = &node;
                break;
            case (u64)GLTFHash::animations:
                nodes.animations = &node;
                break;
        }
    }

#ifdef GLTF
    LOG(OK, "GLTF: '{}'\n", path);
    auto check = [](std::string_view sv, json::Object* p) -> void {
        CERR("\t{}: '{}'\n", sv, p ? p->svKey : "(null)");
    };

    check("scene", this->nodes.scene);
    check("scenes", this->nodes.scenes);
    check("nodes", this->nodes.nodes);
    check("meshes", this->nodes.meshes);
    check("buffers", this->nodes.buffers);
    check("bufferViews", this->nodes.bufferViews);
    check("accessors", this->nodes.accessors);
    check("materials", this->nodes.materials);
    check("textures", this->nodes.textures);
    check("images", this->nodes.images);
    check("samplers", this->nodes.samplers);
    check("skins", this->nodes.skins);
    check("animations", this->nodes.animations);
#endif

    this->defaultSceneIdx = json::getInteger(*this->nodes.scene);

#ifdef GLTF
    LOG(OK, "defaultSceneIdx: {}\n", this->defaultSceneIdx);
    LOG(OK, "processing '{}'...\n", this->nodes.scenes->svKey);
#endif
    {
        /* TODO: push each index (not used much) */
        auto scenes = this->nodes.scenes;
        auto& arr = json::getArray(*scenes);
        for (auto& e : arr)
        {
            auto& obj = json::getObject(e);
            auto pNodes = json::searchObject(obj, "nodes");
            if (pNodes)
            {
                auto& a = json::getArray(*pNodes);
                for (auto& el : a)
                    this->aScenes.push_back({(size_t)json::getInteger(el)});
            }
            else
            {
                this->aScenes.push_back({0});
                break;
            }
        }
    }

#ifdef GLTF
    LOG(OK, "scene nodes: ");
    for (auto& n : this->aScenes)
        CERR("{}, \n", n.nodeIdx);
    CERR("\n");
    LOG(OK, "processing '{}'...\n", this->nodes.buffers->svKey);
#endif
    {
        auto buffers = this->nodes.buffers;
        auto& arr = json::getArray(*buffers);
        for (auto& e : arr)
        {
            auto& obj = json::getObject(e);
            auto pByteLength = json::searchObject(obj, "byteLength");
            auto pUri = json::searchObject(obj, "uri");
            if (!pByteLength) LOG(FATAL, "byteLength field is required\n");

            std::string_view svUri;
            std::vector<char> aBin;

            if (pUri)
            {
                svUri = json::getStringView(*pUri);
                auto sNewPath = replaceFileSuffixInPath(path, svUri);
                aBin = loadFileToCharArray(sNewPath);
            }

            this->aBuffers.push_back({
                .byteLength = (size_t)json::getInteger(*pByteLength),
                .uri = svUri,
                .aBin = aBin
            });
        }
    }
}

} /* namespace gltf */
