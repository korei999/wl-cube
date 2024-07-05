#include <thread>
#include <unordered_map>

#include "model.hh"
#include "parser.hh"

static void parseMtl(std::unordered_map<u64, Materials>* materials, std::string_view path, GLint texMode, App* c);
static void setTanBitan(Vertex* ver1, Vertex* ver2, Vertex* ver3);
    /* copy buffers to the gpu */
static void setBuffers(std::vector<Vertex>* vs, std::vector<GLuint>* els, MeshData* mesh, GLint drawMode, App* c);

enum Hash : u64
{
    comment = hashFNV("#"),
    v = hashFNV("v"),
    vt = hashFNV("vt"),
    vn = hashFNV("vn"),
    f = hashFNV("f"),
    mtllib = hashFNV("mtllib"),
    o = hashFNV("o"),
    usemtl = hashFNV("usemtl"),
    newmtl = hashFNV("newmtl"),
    diff = hashFNV("map_Kd"),
    amb = hashFNV("map_Ka"),
    disp = hashFNV("map_Disp"),
    bump = hashFNV("map_bump"),
    norm = hashFNV("norm")
};

static inline void
bufferData(GLuint* pBO, GLint target, size_t byteLength, void* pData, GLint usage)
{
    glGenBuffers(1, pBO);
    glBindBuffer(target, *pBO);
    glBufferData(target, byteLength, pData, usage);
};

Model::Model(Model&& other)
{
    this->objects = std::move(other.objects);
    this->savedPath = std::move(other.savedPath);
}

Model::Model(std::string_view path, GLint drawMode, GLint texMode, App* c)
{
    loadOBJ(path, drawMode, texMode, c);
}

Model::~Model()
{
    if (this->objects.size())
    {
        for (auto& materials : objects)
            for (auto& mesh : materials)
            {
                glDeleteVertexArrays(1, &mesh.vao);
                glDeleteBuffers(1, &mesh.vbo);
                glDeleteBuffers(1, &mesh.ebo);
            }
    }
}

Model&
Model::operator=(Model&& other)
{
    this->objects = std::move(other.objects);
    this->savedPath = std::move(other.savedPath);
    return *this;
}

void
Model::parseOBJ(std::string_view path, GLint drawMode, GLint texMode, App* c)
{
    GenParser objP(path, " /\n");

    std::vector<v3> vs {};
    std::vector<v2> vts {};
    std::vector<v3> vns {};
    std::string oName {};
    std::string mtllibName {};

    struct FaceData
    {
        int pos[9];

        int& operator[](size_t i) { return pos[i]; }
    };

    struct MaterialData
    {
        std::vector<FaceData> fs;
        std::string usemtl;
    };

    struct Object
    {
        std::vector<MaterialData> mds;
        std::string o; /* object name */
    };

    std::vector<Object> objects;

    auto wordToInt = [](const std::string& str) -> int
    {
        if (str.size() == 0)
            return 0;
        
        return std::stoi(str);
    };

    while (!objP.finished())
    {
        objP.nextWord();

        u64 wordHash = hashFNV(objP.word);
        v3 tv;
        FaceData tf;

        switch (wordHash)
        {
            case Hash::comment:
                objP.skipWord("\n");
                break;

            case Hash::mtllib:
                objP.nextWord("\n");
                mtllibName = objP.word;
                break;

            case Hash::usemtl:
                objP.nextWord("\n");
                objects.back().mds.push_back({});
                objects.back().mds.back().usemtl = objP.word;
                break;

            case Hash::o:
                /* give space for new object */
                objects.push_back({});
                objP.nextWord("\n");

                objects.back().o = objP.word;
                break;

            case Hash::v:
                /* get 3 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);
                objP.nextWord();
                tv.z = std::stof(objP.word);

                vs.push_back(tv);
                break;

            case Hash::vt:
                /* get 2 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);

                vts.push_back(v2(tv));
                break;

            case Hash::vn:
                /* get 3 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);
                objP.nextWord();
                tv.z = std::stof(objP.word);

                vns.push_back(tv);
                break;

            case Hash::f:
                /* get 9 ints */
                objP.nextWord();
                tf[0] = wordToInt(objP.word) - 1; /* obj faces count from 1 */
                objP.nextWord();
                tf[1] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[2] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[3] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[4] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[5] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[6] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[7] = wordToInt(objP.word) - 1;
                objP.nextWord();
                tf[8] = wordToInt(objP.word) - 1;
#ifdef MODEL
                LOG(OK, "f {}/{}/{} {}/{}/{} {}/{}/{}\n", tf[0], tf[1], tf[2], tf[3], tf[4], tf[5], tf[6], tf[7], tf[8]);
#endif
                objects.back().mds.back().fs.push_back(tf);
                break;

            default:
                /* nextlines are empty tokens, skip them */
                break;
        }
    }
    LOG(OK, "vs: {}\tvts: {}\tvns: {}\tobjects: {}\n", vs.size(), vts.size(), vns.size(), objects.size());
#ifdef MODEL
    for (auto& i : objects)
        LOG(OK, "o: '{}', usemtl: '{}'\n", i.o, i.usemtl);
#endif

    /* parse mtl file and load all the textures, later move them to the models */
    std::unordered_map<u64, Materials> materialsMap(objects.size() * 2);

    if (!mtllibName.empty())
    {
        LOG(OK, "loading mtllib: '{}'\n", mtllibName);
        std::string pathToMtl = replaceFileSuffixInPath(path, mtllibName);
        parseMtl(&materialsMap, pathToMtl, texMode, c);
    }

    /* if no textures or normals just add one with zeros */
    if (!vts.size())
        vts.push_back({});
    if (!vns.size())
        vns.push_back({});

    std::unordered_map<FacePositions, GLuint> uniqFaces;
    std::vector<Vertex> verts;
    std::vector<GLuint> inds;
    verts.reserve(vs.size());
    inds.reserve(vs.size());

    // this->meshes.clear();
    for (auto& materials : objects)
    {
        this->objects.push_back({});

        for (auto& faces : materials.mds)
        {
            MeshData mesh {
                .name = materials.o
            };
            GLuint faceIdx = 0;

            for (auto& face : faces.fs)
            {
                /* three vertices for each face */
                constexpr size_t len = LEN(face.pos);
                for (size_t i = 0; i < len; i += 3)
                {
                    FacePositions p {face[i], face[i + 1], face[i + 2]};
                    if (p.y == -1)
                        p.y = 0;

                    auto insTry = uniqFaces.try_emplace(p, faceIdx);
                    if (insTry.second) /* false if we tried to insert duplicate */
                    {
                        /* first v3 positions, second v2 textures, last v3 normals */
                        verts.push_back({vs[p.x], vts[p.y], vns[p.z]});
                        inds.push_back(faceIdx++);
                    }
                    else
                    {
                        inds.push_back(insTry.first->second);
                    }
                }
                /* make tangent and bitangent vectors */
                setTanBitan(&verts[verts.size() - 1], &verts[verts.size() - 2], &verts[verts.size() - 3]);
            }
            setBuffers(&verts, &inds, &mesh, drawMode, c);
            mesh.eboSize = (GLuint)inds.size();

            auto foundTex = materialsMap.find(hashFNV(faces.usemtl));

            this->objects.back().push_back(std::move(mesh));
            this->objects.back().back().materials = std::move(foundTex->second);

            /* TODO: these will be needed later */
            verts.clear();
            inds.clear();
        }
    }

    this->objects.shrink_to_fit();
}

void
Model::loadOBJ(std::string_view path, GLint drawMode, GLint texMode, App* c)
{
    LOG(OK, "loading model: '{}'...\n", path);
    this->parseOBJ(path, drawMode, texMode, c);
    this->savedPath = path;
}

void
Model::loadGLTF(std::string_view path, GLint drawMode, GLint texMode, App* c)
{
    gltf::Asset a(path);

    size_t meshIdx = NPOS;
    for (auto& node : a.aNodes)
    {
        if (node.mesh != NPOS)
        {
            meshIdx = node.mesh;
            break;
        }
    }
    /* get mesh */
    auto& mesh = a.aMeshes[meshIdx];
    auto& primitive = mesh.aPrimitives.front();
    
    size_t indicesAccIdx = primitive.indices;
    size_t positionsAccIdx = primitive.attributes.POSITION;
    size_t normalsAccIdx = primitive.attributes.NORMAL;
    size_t texcoordsAccIdx = primitive.attributes.TEXCOORD_0;
    size_t tangentsAccIdx = primitive.attributes.TANGENT;
    enum gltf::PRIMITIVES mode = primitive.mode;
    this->mode = mode;

    /* indices */
    auto& indAcc = a.aAccessors[indicesAccIdx];
    size_t indBufferView = indAcc.bufferView;
    [[maybe_unused]] size_t indByteOffset = indAcc.byteOffset;
    size_t indCount = indAcc.count;
    gltf::COMPONENT_TYPE indComponentType = indAcc.componentType;
    this->indType = indComponentType;
    this->obj.eboSize = indCount;

    /* positions (should be VEC3 for 3d model) */
    auto& posAcc = a.aAccessors[positionsAccIdx];
    size_t posBufferView = posAcc.bufferView;
    size_t attrPosByteOffset = posAcc.byteOffset;
    [[maybe_unused]] size_t posCount = posAcc.count;

    /* normals */
    auto& normAcc = a.aAccessors[normalsAccIdx];
    size_t normBufferView = normAcc.bufferView;
    size_t attrNormByteOffset = normAcc.byteOffset;
    [[maybe_unused]] size_t normCount = normAcc.count;

    /* textures */
    auto& texAcc = a.aAccessors[texcoordsAccIdx];
    size_t texBufferView = texAcc.bufferView;
    size_t attrTexByteOffset = texAcc.byteOffset;
    enum gltf::COMPONENT_TYPE attrTexComponentType = texAcc.componentType;
    [[maybe_unused]] size_t texCount = texAcc.count;

    /* tangents */
    auto& tanAcc = a.aAccessors[tangentsAccIdx];
    size_t tanBufferView = tanAcc.bufferView;
    size_t attrTanByteOffset = tanAcc.byteOffset;
    [[maybe_unused]] size_t tanCount = tanAcc.count;

    /* indices data */
    auto& bvInd = a.aBufferViews[indBufferView];
    size_t bvIndBufferIdx = bvInd.buffer;
    size_t bvIndByteOffset = bvInd.byteOffset;
    size_t bvIndByteLength = bvInd.byteLength;
    [[maybe_unused]] size_t bvAttrIndByteStride = bvInd.byteStride;
    enum gltf::TARGET bvIndTarget = bvInd.target;

    /* positions data */
    auto& bvPos = a.aBufferViews[posBufferView];
    size_t bvPosBufferIdx = bvPos.buffer;
    size_t bvPosByteOffset = bvPos.byteOffset;
    size_t bvPosByteLength = bvPos.byteLength;
    size_t bvAttrPosByteStride = bvPos.byteStride;
    enum gltf::TARGET bvPosTarget = bvPos.target;

    /* texture coords data */
    auto& bvTex = a.aBufferViews[texBufferView];
    [[maybe_unused]] size_t bvTexBufferIdx = bvTex.buffer;
    [[maybe_unused]] size_t bvTexByteOffset = bvTex.byteOffset;
    [[maybe_unused]] size_t bvTexByteLength = bvTex.byteLength;
    size_t bvAttrTexByteStride = bvTex.byteStride;
    [[maybe_unused]] enum gltf::TARGET bvTexTarget = bvTex.target;

    /* normals data */
    auto& bvNorm = a.aBufferViews[normBufferView];
    [[maybe_unused]] size_t bvNormBufferIdx = bvNorm.buffer;
    [[maybe_unused]] size_t bvNormByteOffset = bvNorm.byteOffset;
    [[maybe_unused]] size_t bvNormByteLength = bvNorm.byteLength;
    size_t bvAttrNormByteStride = bvNorm.byteStride;
    [[maybe_unused]] enum gltf::TARGET bvNormTarget = bvNorm.target;

    /* tangents data */
    auto& bvTan = a.aBufferViews[tanBufferView];
    [[maybe_unused]] size_t bvTanBufferIdx = bvTan.buffer;
    [[maybe_unused]] size_t bvTanByteOffset = bvTan.byteOffset;
    [[maybe_unused]] size_t bvTanByteLength = bvTan.byteLength;
    [[maybe_unused]] size_t bvAttrTanByteStride = bvTan.byteStride;
    [[maybe_unused]] enum gltf::TARGET bvTanTarget = bvTan.target;

    std::lock_guard lock(g_glContextMtx);
    c->bindGlContext();

    glGenVertexArrays(1, &this->obj.vao);
    glBindVertexArray(this->obj.vao);

    bufferData(&this->obj.vbo,
               static_cast<int>(bvPosTarget),
               bvPosByteLength,
               &a.aBuffers[bvPosBufferIdx].aBin.data()[bvPosByteOffset],
               drawMode);

    bufferData(&this->obj.ebo,
               static_cast<int>(bvIndTarget),
               bvIndByteLength,
               &a.aBuffers[bvIndBufferIdx].aBin.data()[bvIndByteOffset],
               drawMode);

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);

    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          v3Size,
                          GL_FLOAT,
                          GL_FALSE,
                          bvAttrPosByteStride,
                          reinterpret_cast<void*>(attrPosByteOffset));
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          v2Size,
                          static_cast<GLenum>(attrTexComponentType),
                          GL_FALSE,
                          bvAttrTexByteStride,
                          reinterpret_cast<void*>(attrTexByteOffset));
    /* normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          v3Size,
                          GL_FLOAT,
                          GL_FALSE,
                          bvAttrNormByteStride,
                          reinterpret_cast<void*>(attrNormByteOffset));
    /* tangents */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,
                          v3Size,
                          GL_FLOAT,
                          GL_FALSE,
                          bvAttrTanByteStride,
                          reinterpret_cast<void*>(attrTanByteOffset));

    glBindVertexArray(0);
    c->unbindGlContext();
}

static void
setBuffers(std::vector<Vertex>* verts, std::vector<GLuint>* inds, MeshData* m, GLint drawMode, App* c)
{
    std::lock_guard lock(g_glContextMtx);

    c->bindGlContext();

    auto vsData = verts->data();
    auto inData = inds->data();

    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    glGenBuffers(1, &m->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds->size() * sizeof(*inData), inData, drawMode);

    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, verts->size() * sizeof(*vsData), vsData, drawMode);

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(f32) * v3Size));
    /* normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, v3Size, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(f32) * (v3Size + v2Size)));
    /* tangents */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, v3Size, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(f32) * (2*v3Size + v2Size)));
    /* bitangents */
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, v3Size, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(f32) * (3*v3Size + v2Size)));

    glBindVertexArray(0);

    c->unbindGlContext();
}

void
Model::draw()
{
    for (auto& meshes : objects)
        for (auto& mesh : meshes)
        {
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr);
        }
}

void
Model::drawGLTF()
{
    glBindVertexArray(this->obj.vao);
    glDrawElements(static_cast<GLenum>(this->mode), this->obj.eboSize, static_cast<GLenum>(this->indType), nullptr);
}

void
Model::drawInstanced(GLsizei count)
{
    for (auto& meshes : objects)
        for (auto& mesh : meshes)
        {
            glBindVertexArray(mesh.vao);
            glDrawElementsInstanced(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr, count);
        }
}

void
Model::drawTex(GLint primitives)
{
    for (auto& materials : objects)
        for (auto& mesh : materials)
        {
            mesh.materials.diffuse.bind(GL_TEXTURE0);

            glBindVertexArray(mesh.vao);
            glDrawElements(primitives, mesh.eboSize, GL_UNSIGNED_INT, nullptr);
        }
}

Ubo::Ubo(size_t _size, GLint drawMode)
{
    createBuffer(_size, drawMode);
}

Ubo::~Ubo()
{
    glDeleteBuffers(1, &id);
    LOG(OK, "ubo '{}' deleted\n", id);
}

void
Ubo::createBuffer(size_t _size, GLint drawMode)
{
    size = _size;
    glGenBuffers(1, &id);
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, drawMode);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void
Ubo::bindBlock(Shader* sh, std::string_view block, GLuint _point)
{
    point = _point;
    GLuint index;
    index = glGetUniformBlockIndex(sh->id, block.data());
    glUniformBlockBinding(sh->id, index, _point);
    LOG(OK, "uniform block: '{}' at '{}', in shader '{}'\n", block, index, sh->id);

    glBindBufferBase(GL_UNIFORM_BUFFER, _point, id);
    /* or */
    // glBindBufferRange(GL_UNIFORM_BUFFER, point, id, 0, size);
}

void
Ubo::bufferData(void* data, size_t offset, size_t _size)
{
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, _size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Model
getQuad(GLint drawMode)
{
    f32 quadVertices[] {
        -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
         1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
    };

    GLuint quadIndices[] {
        0, 1, 2, 0, 2, 3
    };

    Model q;
    q.objects.resize(1);
    q.objects.back().resize(1);

    glGenVertexArrays(1, &q.objects[0][0].vao);
    glBindVertexArray(q.objects[0][0].vao);

    glGenBuffers(1, &q.objects[0][0].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q.objects[0][0].vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, drawMode);

    glGenBuffers(1, &q.objects[0][0].ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q.objects[0][0].ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, drawMode);
    q.objects[0][0].eboSize = LEN(quadIndices);

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
    constexpr size_t stride = 5 * sizeof(f32);
    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, stride, (void*)0);
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(f32) * v3Size));

    glBindVertexArray(0);

    LOG(OK, "quad '{}' created\n", q.objects[0][0].vao);
    q.savedPath = "Quad";
    return q;
}

Model
getPlane(GLint drawMode)
{
    f32 planeVertices[] {
        /* positions            texcoords   normals          */
         25.0f, -0.5f,  25.0f,  25.0f,  0.0f,  0.0f, 1.0f, 0.0f,  
        -25.0f, -0.5f, -25.0f,   0.0f, 25.0f,  0.0f, 1.0f, 0.0f,  
        -25.0f, -0.5f,  25.0f,   0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  
                                                                  
        -25.0f, -0.5f, -25.0f,   0.0f, 25.0f,  0.0f, 1.0f, 0.0f,  
         25.0f, -0.5f,  25.0f,  25.0f,  0.0f,  0.0f, 1.0f, 0.0f,  
         25.0f, -0.5f, -25.0f,  25.0f, 25.0f,  0.0f, 1.0f, 0.0f 
    };

    Model q;
    q.objects.resize(1);
    q.objects.back().resize(1);

    glGenVertexArrays(1, &q.objects[0][0].vao);
    glBindVertexArray(q.objects[0][0].vao);

    glGenBuffers(1, &q.objects[0][0].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q.objects[0][0].vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, drawMode);

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
    constexpr size_t stride = 8 * sizeof(f32);
    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, stride, (void*)0);
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(f32) * v3Size));
    /* normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, v3Size, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(f32) * (v3Size + v2Size)));

    glBindVertexArray(0);

    LOG(OK, "plane '{}' created\n", q.objects[0][0].vao);
    return q;
}

void
drawQuad(const Model& q)
{
    glBindVertexArray(q.objects[0][0].vao);
    glDrawElements(GL_TRIANGLES, q.objects[0][0].eboSize, GL_UNSIGNED_INT, nullptr);
}

void
drawPlane(const Model& q)
{
    glBindVertexArray(q.objects[0][0].vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Model
getCube(GLint drawMode)
{
    float cubeVertices[] {
        /* back face */
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, /* bottom-left */
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, /* top-right */
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, /* bottom-right */    
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, /* top-right */
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, /* bottom-left */
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, /* top-left */
        /* front face */
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, /* bottom-left */
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, /* bottom-right */
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, /* top-right */
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, /* top-right */
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, /* top-left */
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, /* bottom-left */
        /* left face */
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, /* top-right */
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, /* top-left */
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, /* bottom-left */
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, /* bottom-left */
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, /* bottom-right */
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, /* top-right */
        /* right face */
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, /* top-left */
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, /* bottom-right */
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, /* top-right */
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, /* bottom-right */
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, /* top-left */
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, /* bottom-left */
        /* bottom face */
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, /* top-right */
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, /* top-left */
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, /* bottom-le ft */
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, /* bottom-le ft */
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, /* bottom-ri ght */
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, /* top-right */
        /* top face */
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, /* top-left */
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, /* bottom-right */
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, /* top-right */
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, /* bottom-right */
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, /* top-left */
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  /* bottom-left */
    };

    Model q;
    q.objects.resize(1);
    q.objects.back().resize(1);

    glGenVertexArrays(1, &q.objects[0][0].vao);
    glBindVertexArray(q.objects[0][0].vao);

    glGenBuffers(1, &q.objects[0][0].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q.objects[0][0].vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, drawMode);

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
    constexpr size_t stride = 8 * sizeof(f32);

    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, stride, (void*)0);
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(f32) * v3Size));
    /* normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, v3Size, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(f32) * (v3Size + v2Size)));

    glBindVertexArray(0);

    LOG(OK, "cube '{}' created\n", q.objects[0][0].vao);
    return q;
}

void
drawCube(const Model& q)
{
    glBindVertexArray(q.objects[0][0].vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void
parseMtl(std::unordered_map<u64, Materials>* materials, std::string_view path, GLint texMode, App* c)
{
    GenParser p(path, " \n");
    decltype(materials->insert({u64(), Materials()})) ins; /* get iterator placeholder */

    std::vector<std::jthread> threads;

    while (!p.finished())
    {
        p.nextWord();

        u64 wordHash = hashFNV(p.word);

        switch (wordHash)
        {
            case Hash::comment:
                p.skipWord("\n");
                break;

            case Hash::newmtl:
                p.nextWord("\n");
                ins = materials->insert({hashFNV(p.word), {}});
                break;

            case Hash::diff:
                p.nextWord("\n");
                /* TODO: implement thread pool for this kind of stuff */
                threads.emplace_back(&Texture::loadBMP,
                                     &ins.first->second.diffuse,
                                     replaceFileSuffixInPath(path, p.word),
                                     TEX_TYPE::diffuse,
                                     false,
                                     texMode,
                                     c);
                break;

            case Hash::bump:
            case Hash::norm:
                p.nextWord("\n");
                threads.emplace_back(&Texture::loadBMP,
                                     &ins.first->second.normal,
                                     replaceFileSuffixInPath(path, p.word),
                                     TEX_TYPE::normal,
                                     false,
                                     texMode,
                                     c);
                break;

            default:
                break;
        }
    }
}

static void
setTanBitan(Vertex* ver0, Vertex* ver1, Vertex* ver2)
{
    v3 edge0 = ver1->pos - ver0->pos;
    v3 edge1 = ver2->pos - ver0->pos;

    v2 deltaUV0 = ver1->tex - ver0->tex;
    v2 deltaUV1 = ver2->tex - ver0->tex;

    f32 invDet = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

    ver0->tan = ver1->tan = ver2->tan = v3(
        invDet * (deltaUV1.y * edge0.x - deltaUV0.y * edge1.x),
        invDet * (deltaUV1.y * edge0.y - deltaUV0.y * edge1.y),
        invDet * (deltaUV1.y * edge0.z - deltaUV0.y * edge1.z)
    );
    ver0->bitan = ver1->bitan = ver2->bitan = v3(
        invDet * (-deltaUV1.x * edge0.x + deltaUV0.x * edge1.x),
        invDet * (-deltaUV1.x * edge0.y + deltaUV0.x * edge1.y),
        invDet * (-deltaUV1.x * edge0.z + deltaUV0.x * edge1.z)
    );
}
