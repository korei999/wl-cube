#include "gltf.hh"
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

static inline std::string_view
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

static inline std::string_view
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

static inline std::string_view
getPrimitiveModeString(enum PRIMITIVE_MODE pm)
{
    constexpr std::string_view ss[] {
        "POINTS", "LINES", "LINE_LOOP", "LINE_STRIP", "TRIANGLES", "TRIANGLE_STRIP", "TRIANGLE_FAN"
    };

    return ss[static_cast<int>(pm)];
}

static inline enum ACCESSOR_TYPE
stringToAccessorType(std::string_view sv)
{
    switch (hashFNV(sv))
    {
        default:
        case static_cast<u64>(HASH_CODES::SCALAR):
            return ACCESSOR_TYPE::SCALAR;
        case static_cast<u64>(HASH_CODES::VEC2):
            return ACCESSOR_TYPE::VEC2;
        case static_cast<u64>(HASH_CODES::VEC3):
            return ACCESSOR_TYPE::VEC3;
        case static_cast<u64>(HASH_CODES::VEC4):
            return ACCESSOR_TYPE::VEC4;
        case static_cast<u64>(HASH_CODES::MAT3):
            return ACCESSOR_TYPE::MAT3;
        case static_cast<u64>(HASH_CODES::MAT4):
            return ACCESSOR_TYPE::MAT4;
    }
}

static inline std::string_view
accessorTypeToString(enum ACCESSOR_TYPE t)
{
    constexpr std::string_view ss[] {
        "SCALAR", "VEC2", "VEC3", "VEC4", /*MAT2, Unused*/ "MAT3", "MAT4"
    };
    return ss[static_cast<int>(t)];
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

static inline std::string
getUnionTypeString(enum ACCESSOR_TYPE type, const union Type& t, std::string_view prefix)
{
    switch (type)
    {
        default:
            return "unknown";
        case ACCESSOR_TYPE::SCALAR:
            return FMT("{}({})", prefix, t.SCALAR);
        case ACCESSOR_TYPE::VEC2:
            return FMT("{}({}, {})", prefix, t.VEC2.x, t.VEC2.y);
        case ACCESSOR_TYPE::VEC3:
            return FMT("{}({}, {}, {})", prefix, t.VEC3.x, t.VEC3.y, t.VEC3.z);
        case ACCESSOR_TYPE::VEC4:
            return FMT("{}({}, {}, {}, {})", prefix, t.VEC4.x, t.VEC4.y, t.VEC4.z, t.VEC4.w);
        case ACCESSOR_TYPE::MAT3:
            return FMT("{}({}, {}, {}\n"
                       "{} {}, {}, {}\n"
                       "{} {}, {}, {})\n", prefix, t.MAT3.e[0][0], t.MAT3.e[0][1], t.MAT3.e[0][2],
                                           prefix, t.MAT3.e[1][0], t.MAT3.e[1][1], t.MAT3.e[1][2],
                                           prefix, t.MAT3.e[2][0], t.MAT3.e[2][1], t.MAT3.e[2][2]);
        case ACCESSOR_TYPE::MAT4:
            return FMT("{}({}, {}, {}, {}\n"
                       "{} {}, {}, {}, {}\n"
                       "{} {}, {}, {}, {}\n"
                       "{} {}, {}, {}, {})\n", prefix, t.MAT4.e[0][0], t.MAT4.e[0][1], t.MAT4.e[0][2], t.MAT4.e[0][3],
                                               prefix, t.MAT4.e[1][0], t.MAT4.e[1][1], t.MAT4.e[1][2], t.MAT4.e[1][3],
                                               prefix, t.MAT4.e[2][0], t.MAT4.e[2][1], t.MAT4.e[2][2], t.MAT4.e[2][3],
                                               prefix, t.MAT4.e[3][0], t.MAT4.e[3][1], t.MAT4.e[3][2], t.MAT4.e[3][3]);

    }
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
            case static_cast<u64>(HASH_CODES::scene):
                nodes.scene = &node;
                break;
            case static_cast<u64>(HASH_CODES::scenes):
                nodes.scenes = &node;
                break;
            case static_cast<u64>(HASH_CODES::nodes):
                nodes.nodes = &node;
                break;
            case static_cast<u64>(HASH_CODES::meshes):
                nodes.meshes = &node;
                break;
            case static_cast<u64>(HASH_CODES::cameras):
                nodes.cameras = &node;
                break;
            case static_cast<u64>(HASH_CODES::buffers):
                nodes.buffers = &node;
                break;
            case static_cast<u64>(HASH_CODES::bufferViews):
                nodes.bufferViews = &node;
                break;
            case static_cast<u64>(HASH_CODES::accessors):
                nodes.accessors = &node;
                break;
            case static_cast<u64>(HASH_CODES::materials):
                nodes.materials = &node;
                break;
            case static_cast<u64>(HASH_CODES::textures):
                nodes.textures = &node;
                break;
            case static_cast<u64>(HASH_CODES::images):
                nodes.images = &node;
                break;
            case static_cast<u64>(HASH_CODES::samplers):
                nodes.samplers = &node;
                break;
            case static_cast<u64>(HASH_CODES::skins):
                nodes.skins = &node;
                break;
            case static_cast<u64>(HASH_CODES::animations):
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
             bv.buffer, bv.byteOffset, bv.byteLength, bv.byteStride, getTargetString(bv.target));

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

#ifdef GLTF
    LOG(OK, "accessors:\n");
    for (auto& a : this->aAccessors)
    {
        CERR("\tbufferView: '{}'\n\tbyteOffset: '{}'\n\tcomponentType: '{}'\n\tcount: '{}'\n",
             a.bufferView, a.byteOffset, getComponentTypeString(a.componentType), a.count);
        CERR("\tmax:\n{}\n", getUnionTypeString(a.type, a.max, "\t"));
        CERR("\tmin:\n{}\n", getUnionTypeString(a.type, a.min, "\t"));
        CERR("\ttype: '{}'\n\n", accessorTypeToString(a.type));
    }

    LOG(OK, "processing '{}'...\n", this->nodes.meshes->svKey);
#endif
    {
        auto meshes = this->nodes.meshes;
        auto& arr = json::getArray(*meshes);
        for (auto& e : arr)
        {
            auto& obj = json::getObject(e);

            auto pPrimitives = json::searchObject(obj, "primitives");
            if (!pPrimitives) LOG(FATAL, "'primitives' field is required\n");

            std::vector<Primitive> aPrimitives;
            auto pName = json::searchObject(obj, "name");
            auto name = pName ? json::getStringView(*pName) : "";

            auto& aPrim = json::getArray(*pPrimitives);
            for (auto& p : aPrim)
            {
                auto& op = json::getObject(p);

                auto pAttributes = json::searchObject(op, "attributes");
                auto& oAttr = json::getObject(*pAttributes);
                auto pNORMAL = json::searchObject(oAttr, "NORMAL");
                auto pTANGENT = json::searchObject(oAttr, "TANGENT");
                auto pPOSITION = json::searchObject(oAttr, "POSITION");
                auto pTEXCOORD_0 = json::searchObject(oAttr, "TEXCOORD_0");

                auto pIndices = json::searchObject(op, "indices");
                auto pMode = json::searchObject(op, "mode");
                auto pMaterial = json::searchObject(op, "material");

                aPrimitives.push_back({
                    .attributes {
                        .NORMAL = pNORMAL ? static_cast<decltype(Primitive::attributes.NORMAL)>(json::getInteger(*pNORMAL)) : 0,
                        .POSITION = pPOSITION ? static_cast<decltype(Primitive::attributes.POSITION)>(json::getInteger(*pPOSITION)) : 0,
                        .TEXCOORD_0 = pTEXCOORD_0 ? static_cast<decltype(Primitive::attributes.TEXCOORD_0)>(json::getInteger(*pTEXCOORD_0)) : 0,
                        .TANGENT = pTANGENT ? static_cast<decltype(Primitive::attributes.TANGENT)>(json::getInteger(*pTANGENT)) : 0,
                    },
                    .indices = pIndices ? static_cast<decltype(Primitive::indices)>(json::getInteger(*pIndices)) : 0,
                    .material = pMaterial ? static_cast<decltype(Primitive::material)>(json::getInteger(*pMaterial)) : 0,
                    .mode = pMode ? static_cast<decltype(Primitive::mode)>(json::getInteger(*pMode)) : PRIMITIVE_MODE::TRIANGLES,
                });
            }

            this->aMeshes.push_back({.aPrimitives = aPrimitives, .svName = name});
        }
    }
#ifdef GLTF
    LOG(OK, "meshes:\n");
    for (auto& m : this->aMeshes)
    {
        CERR("\tname: '{}'\n", m.svName);
        for (auto& p : m.aPrimitives)
        {
            CERR("\tattributes:\n");
            CERR("\t\tNORMAL: '{}', POSITION: '{}', TEXCOORD_0: '{}', TANGENT: '{}'\n",
                 p.attributes.NORMAL, p.attributes.POSITION, p.attributes.TEXCOORD_0, p.attributes.TANGENT);
            CERR("\tindices: '{}', material: '{}, mode: '{}''\n\n", p.indices, p.material, getPrimitiveModeString(p.mode));
        }
    }
#endif
}

} /* namespace gltf */
