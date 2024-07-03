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
    for (auto& node : json::getObject(this->p.m_upHead->tagVal.val))
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
    auto check = [](std::string_view sv, json::KeyVal* p) -> void {
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

    this->defaultSceneIdx = json::getInteger(this->nodes.scene->tagVal.val);

#ifdef GLTF
    LOG(OK, "defaultSceneIdx: {}\n", this->defaultSceneIdx);
    LOG(OK, "processing '{}'...\n", this->nodes.scenes->svKey);
#endif
    {
        /* TODO: push each index (not used much) */
        auto scenes = this->nodes.scenes;
        auto& arr = json::getArray(scenes->tagVal.val);
        auto& obj = json::getObject(arr.front().val);
        auto& n0 = json::getObject(obj.front().tagVal.val);
        auto& arrN0 = json::getArray(n0.front().tagVal.val);
        auto number = json::getInteger(arrN0.front().val);
        this->aScenes.push_back({(size_t)number});
    }
#ifdef GLTF
    LOG(OK, "first scene idx: '{}'\n", this->aScenes.front().nodeIdx);
    LOG(OK, "processing '{}'...\n", this->nodes.buffers->svKey);
#endif
    {
        auto buffs = this->nodes.buffers;
        auto& arr = json::getArray(buffs->tagVal.val);
        for (size_t i = 0; i < arr.size(); i++)
        {
            auto& obj = json::getObject(arr[i].val);
            for (size_t j = 0; j < obj.size(); j++)
            {
                /* each key/value pair can also be an object(array), so get it once more */
                auto& o = json::getObject(obj[j].tagVal.val);
                auto uri = json::searchObject(o, "uri");
                auto byteLength = json::searchObject(o, "byteLength");
                if (!byteLength) LOG(FATAL, "byteLength field is required\n");

                if (uri)
                {
                    auto sv = json::getStringView(uri->tagVal.val);
                    auto newPath = replaceFileSuffixInPath(path, sv);
                    LOG(OK, "bin path: '{}'\n", newPath);
                    auto file = loadFileToCharArray(newPath);
                    size_t len = json::getInteger(byteLength->tagVal.val);

                    LOG(OK, "byteLength: '{}', uri: '{}'\n", len, sv);
                    this->aBuffers.push_back({.byteLength = len, .uri = sv, .aBin = std::move(file)});
                }
            }
        }
    }
#ifdef GLTF
    LOG(OK, "processing '{}'...\n", this->nodes.bufferViews->svKey);
#endif
    {
        auto& views = this->nodes.bufferViews;
        auto& arr = json::getArray(views->tagVal.val);
        for (auto& e : arr)
        {
            auto& obj = json::getObject(e.val);
            for (auto& ob : obj)
            {
                auto& o = json::getObject(ob.tagVal.val);
                auto pBuffer = json::searchObject(o, "buffer");
                if (!pBuffer) LOG(FATAL, "buffer field is required\n");

                auto pByteOffset = json::searchObject(o, "byteOffset");
                auto pByteLength = json::searchObject(o, "byteLength");
                if (!pByteLength) LOG(FATAL, "byteLength field is required\n");
                auto pByteStride = json::searchObject(o, "byteStride");
                auto pTarget = json::searchObject(o, "target");

                size_t buffer = json::getInteger(pBuffer->tagVal.val);;
                size_t byteOffset = pByteOffset ? json::getInteger(pByteOffset->tagVal.val) : 0;
                size_t byteLength = pByteLength ? json::getInteger(pByteLength->tagVal.val) : 0;
                size_t byteStride = pByteStride ? json::getInteger(pByteStride->tagVal.val) : 0;
                enum TARGET target = pTarget ? (TARGET)json::getInteger(pTarget->tagVal.val) : TARGET::NONE;

                this->aBufferViews.push_back({
                    .buffer = buffer,
                    .byteOffset = byteOffset,
                    .byteLength = byteLength,
                    .byteStride = byteStride,
                    .target = target
                });
            }
        }
    }
#ifdef GLTF
    for (size_t i = 0; i < this->aBufferViews.size(); i++)
    {
        const auto& bv = this->aBufferViews[i];
        LOG(OK, "[{}]: buffer: {}, byteOffset: {}, byteLength: {}, byteStride: {}, target: {}\n",
            i, bv.buffer, bv.byteOffset, bv.byteLength, bv.byteStride, (long)bv.target); 
    }
#endif
}

} /* namespace gltf */
