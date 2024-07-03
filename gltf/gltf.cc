#include "gltf.hh"

namespace gltf
{

static inline enum ACCESSOR_TYPE
stringToAccessorType(std::string_view sv)
{
    switch (hashFNV(sv))
    {
        default:
        case (u64)HASH_CODES::SCALAR:
            return ACCESSOR_TYPE::SCALAR;
        case (u64)HASH_CODES::VEC2:
            return ACCESSOR_TYPE::VEC2;
        case (u64)HASH_CODES::VEC3:
            return ACCESSOR_TYPE::VEC3;
        case (u64)HASH_CODES::VEC4:
            return ACCESSOR_TYPE::VEC4;
        case (u64)HASH_CODES::MAT3:
            return ACCESSOR_TYPE::MAT3;
        case (u64)HASH_CODES::MAT4:
            return ACCESSOR_TYPE::MAT4;
    }
}

static inline union Type
accessorTypeToUnionType(enum ACCESSOR_TYPE t, json::Object* obj)
{
    union Type type;

    auto assignUnionType = [](json::Object* obj, size_t n) -> union Type {
        auto& arr = json::getArray(*obj);
        union Type type;

        for (size_t i = 0; i < n; i++)
            type.MAT4.p[i] = json::getReal(arr[i]);

        return type;
    };

    switch (t)
    {
        default:
        case ACCESSOR_TYPE::SCALAR:
            {
                auto& arr = json::getArray(*obj);
                type.SCALAR = static_cast<size_t>(json::getInteger(arr[0]));
            }
            break;
        case ACCESSOR_TYPE::VEC2:
            type = assignUnionType(obj, 2);
            break;
        case ACCESSOR_TYPE::VEC3:
            type = assignUnionType(obj, 3);
            break;
        case ACCESSOR_TYPE::VEC4:
            type = assignUnionType(obj, 4);
            break;
        case ACCESSOR_TYPE::MAT3:
            type = assignUnionType(obj, 3*3);
            break;
        case ACCESSOR_TYPE::MAT4:
            type = assignUnionType(obj, 4*4);
            break;
    }

    return type;
}

Asset::Asset(std::string_view path)
    : p(path)
{
    p.parse();

    /* collect all the top level objects */
    for (auto& node : json::getObject(*this->p.m_upHead))
    {
        switch (hashFNV(node.svKey))
        {
            default:
                break;
            case (u64)HASH_CODES::scene:
                nodes.scene = &node;
                break;
            case (u64)HASH_CODES::scenes:
                nodes.scenes = &node;
                break;
            case (u64)HASH_CODES::nodes:
                nodes.nodes = &node;
                break;
            case (u64)HASH_CODES::meshes:
                nodes.meshes = &node;
                break;
            case (u64)HASH_CODES::cameras:
                nodes.cameras = &node;
                break;
            case (u64)HASH_CODES::buffers:
                nodes.buffers = &node;
                break;
            case (u64)HASH_CODES::bufferViews:
                nodes.bufferViews = &node;
                break;
            case (u64)HASH_CODES::accessors:
                nodes.accessors = &node;
                break;
            case (u64)HASH_CODES::materials:
                nodes.materials = &node;
                break;
            case (u64)HASH_CODES::textures:
                nodes.textures = &node;
                break;
            case (u64)HASH_CODES::images:
                nodes.images = &node;
                break;
            case (u64)HASH_CODES::samplers:
                nodes.samplers = &node;
                break;
            case (u64)HASH_CODES::skins:
                nodes.skins = &node;
                break;
            case (u64)HASH_CODES::animations:
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
        CERR("{}, ", n.nodeIdx);
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
            if (!pByteLength) LOG(FATAL, "'byteLength' field is required\n");

            std::string_view svUri;
            std::vector<char> aBin;

            if (pUri)
            {
                svUri = json::getStringView(*pUri);
                auto sNewPath = replaceFileSuffixInPath(path, svUri);
                aBin = loadFileToCharArray(sNewPath);
            }

            this->aBuffers.push_back({
                .byteLength = static_cast<size_t>(json::getInteger(*pByteLength)),
                .uri = svUri,
                .aBin = aBin
            });
        }
    }
#ifdef GLTF
    LOG(OK, "buffers:\n");
    for (auto& b : this->aBuffers)
        CERR("\tbyteLength: '{}', uri: '{}'\n", b.byteLength, b.uri);

    LOG(OK, "processing '{}'...\n", this->nodes.bufferViews->svKey);
#endif
    {
        auto bufferViews = this->nodes.bufferViews;
        auto& arr = json::getArray(*bufferViews);
        for (auto& e : arr)
        {
            auto& obj = json::getObject(e);

            auto pBuffer = json::searchObject(obj, "buffer");
            if (!pBuffer) LOG(FATAL, "'buffer' field is required\n");
            auto pByteOffset = json::searchObject(obj, "byteOffset");
            auto pByteLength = json::searchObject(obj, "byteLength");
            if (!pByteLength) LOG(FATAL, "'byteLength' field is required\n");
            auto pByteStride = json::searchObject(obj, "byteStride");
            auto pTarget = json::searchObject(obj, "target");

            this->aBufferViews.push_back({
                .buffer = static_cast<size_t>(json::getInteger(*pBuffer)),
                .byteOffset = pByteOffset ? static_cast<size_t>(json::getInteger(*pByteOffset)) : 0,
                .byteLength = static_cast<size_t>(json::getInteger(*pByteLength)),
                .byteStride = pByteStride ? static_cast<size_t>(json::getInteger(*pByteStride)) : 0,
                .target = pTarget ? static_cast<enum TARGET>(json::getInteger(*pTarget)) : TARGET::NONE
            });
        }
    }
#ifdef GLTF
    LOG(OK, "bufferViews:\n");
    for (auto& bv : this->aBufferViews)
        CERR("\tbuffer: '{}'\n\tbyteOffset: '{}'\n\tbyteLength: '{}'\n\tbyteStride: '{}'\n\ttarget: '{}'\n\n",
             bv.buffer, bv.byteOffset, bv.byteLength, bv.byteStride, getTARGETString(bv.target));

    LOG(OK, "processing '{}'...\n", this->nodes.accessors->svKey);
#endif
    {
        auto accessors = this->nodes.accessors;
        auto& arr = json::getArray(*accessors);
        for (auto& e : arr)
        {
            auto& obj = json::getObject(e);

            auto pBufferView = json::searchObject(obj, "bufferView");
            auto pByteOffset = json::searchObject(obj, "byteOffset");
            auto pComponentType = json::searchObject(obj, "componentType");
            if (!pComponentType) LOG(FATAL, "'componentType' filed is required\n");
            auto pCount = json::searchObject(obj, "count");
            if (!pCount) LOG(FATAL, "'count' field is required\n");
            auto pMax = json::searchObject(obj, "max");
            auto pMin = json::searchObject(obj, "min");
            auto pType = json::searchObject(obj, "type");
            if (!pType) LOG(FATAL, "'type' field is required\n");

            enum ACCESSOR_TYPE type = stringToAccessorType(json::getStringView(*pType));

            this->aAccessors.push_back({
                .bufferView = pBufferView ? static_cast<size_t>(json::getInteger(*pBufferView)) : 0,
                .byteOffset = pByteOffset ? static_cast<size_t>(json::getInteger(*pByteOffset)) : 0,
                .componentType = static_cast<enum COMPONENT_TYPE>(json::getInteger(*pComponentType)),
                .count = static_cast<size_t>(json::getInteger(*pCount)),
                .max = pMax ? accessorTypeToUnionType(type, pMax) : Type{},
                .min = pMin ? accessorTypeToUnionType(type, pMin) : Type{},
                .type = type
            });
        }
    }
}

} /* namespace gltf */
