#include <thread>
#include <unordered_map>

#include "model.hh"
#include "parser.hh"

static void parseMtl(std::unordered_map<u64, Materials>* materials, std::string_view path, GLint texMode, App* c);
static void setTanBitan(Vertex* ver1, Vertex* ver2, Vertex* ver3);
    /* copy buffers to the gpu */
static void setBuffers(std::vector<Vertex>* vs, std::vector<GLuint>* els, MeshData* mesh, GLint drawMode, App* c);

enum HASH : u64
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
            case HASH::comment:
                objP.skipWord("\n");
                break;

            case HASH::mtllib:
                objP.nextWord("\n");
                mtllibName = objP.word;
                break;

            case HASH::usemtl:
                objP.nextWord("\n");
                objects.back().mds.push_back({});
                objects.back().mds.back().usemtl = objP.word;
                break;

            case HASH::o:
                /* give space for new object */
                objects.push_back({});
                objP.nextWord("\n");

                objects.back().o = objP.word;
                break;

            case HASH::v:
                /* get 3 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);
                objP.nextWord();
                tv.z = std::stof(objP.word);

                vs.push_back(tv);
                break;

            case HASH::vt:
                /* get 2 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);

                vts.push_back(v2(tv));
                break;

            case HASH::vn:
                /* get 3 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);
                objP.nextWord();
                tv.z = std::stof(objP.word);

                vns.push_back(tv);
                break;

            case HASH::f:
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
        std::string pathToMtl = replacePathSuffix(path, mtllibName);
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
            MeshData mesh {};
            mesh.name = materials.o;
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
                        verts.push_back({vs[p.x], vts[p.y], vns[p.z], {}, {}});
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
    this->asset.load(path);
    auto& a = this->asset;;

    /* load buffers first */
    std::vector<GLuint> aBufferMap;
    for (size_t i = 0; i < a.aBuffers.size(); i++)
    {
        std::scoped_lock lock(g_mtxGlContext);
        c->bindGlContext();

        GLuint b;
        glGenBuffers(1, &b);
        glBindBuffer(GL_ARRAY_BUFFER, b);
        glBufferData(GL_ARRAY_BUFFER, a.aBuffers[i].byteLength, a.aBuffers[i].aBin.data(), drawMode);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        aBufferMap.push_back(b);

        c->unbindGlContext();
    }

    /* TODO: preload textures + map duplicates */

    size_t meshIdx = NPOS;
    for (auto& node : a.aNodes)
    {
        if (node.mesh == NPOS)
            continue;

        meshIdx = node.mesh;

        auto& mesh = a.aMeshes[meshIdx];
        for (auto& primitive : mesh.aPrimitives)
        {
            size_t accIndIdx = primitive.indices;
            size_t accPosIdx = primitive.attributes.POSITION;
            size_t accNormIdx = primitive.attributes.NORMAL;
            size_t accTexIdx = primitive.attributes.TEXCOORD_0;
            size_t accTanIdx = primitive.attributes.TANGENT;
            size_t accMatIdx = primitive.material;
            enum gltf::PRIMITIVES mode = primitive.mode;

            auto& accPos = a.aAccessors[accPosIdx];
            auto& accTex = a.aAccessors[accTexIdx];
            auto& accTan = a.aAccessors[accTanIdx];

            auto& bvPos = a.aBufferViews[accPos.bufferView];
            auto& bvTex = a.aBufferViews[accTex.bufferView];

            Mesh2 nMesh2 {};
            nMesh2.mode = mode;
            nMesh2.vScale = node.scale;

            /* manually unlock before loading texture */
            g_mtxGlContext.lock();
            c->bindGlContext();

            glGenVertexArrays(1, &nMesh2.meshData.vao);
            glBindVertexArray(nMesh2.meshData.vao);

            if (accIndIdx != NPOS)
            {
                auto& accInd = a.aAccessors[accIndIdx];
                auto& bvInd = a.aBufferViews[accInd.bufferView];
                nMesh2.indType = accInd.componentType;
                nMesh2.meshData.eboSize = accInd.count;
                nMesh2.triangleCount = NPOS;

                /* TODO: figure out how to reuse VBO data for index buffer (possible?) */
                glGenBuffers(1, &nMesh2.meshData.ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nMesh2.meshData.ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, bvInd.byteLength,
                             &a.aBuffers[bvInd.buffer].aBin.data()[bvInd.byteOffset + accInd.byteOffset], drawMode);
            }
            else
            {
                nMesh2.triangleCount = accPos.count;
            }

            constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
            constexpr size_t v2Size = sizeof(v2) / sizeof(f32);

            /* if there are different VBO's for positions textures or normals,
             * given gltf file should be considered harmful, and this will crash ofc */
            nMesh2.meshData.vbo = aBufferMap[bvPos.buffer];
            glBindBuffer(GL_ARRAY_BUFFER, nMesh2.meshData.vbo);

            /* positions */
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, v3Size, static_cast<GLenum>(accPos.componentType), GL_FALSE,
                                  bvPos.byteStride, reinterpret_cast<void*>(bvPos.byteOffset + accPos.byteOffset));

            /* texture coords */
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, v2Size, static_cast<GLenum>(accTex.componentType), GL_FALSE,
                                  bvTex.byteStride, reinterpret_cast<void*>(bvTex.byteOffset + accTex.byteOffset));

             /*normals */
            if (accNormIdx != NPOS)
            {
                auto& accNorm = a.aAccessors[accNormIdx];
                auto& bvNorm = a.aBufferViews[accNorm.bufferView];

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, v3Size, static_cast<GLenum>(accNorm.componentType), GL_FALSE,
                                      bvNorm.byteStride, reinterpret_cast<void*>(accNorm.byteOffset + bvNorm.byteOffset));
            }

            /* tangents */
            /*auto& bvTan = a.aBufferViews[accTan.bufferView];*/
            /*glEnableVertexAttribArray(3);*/
            /*glVertexAttribPointer(3, v3Size, static_cast<GLenum>(accTan.componentType), GL_FALSE,*/
            /*                      bvTan.byteStride, reinterpret_cast<void*>(accTan.byteOffset + bvTan.byteOffset));*/

            glBindVertexArray(0);
            c->unbindGlContext();
            g_mtxGlContext.unlock();

            /* load textures */
            if (accMatIdx != NPOS)
            {
                auto& mat = a.aMaterials[accMatIdx];
                size_t baseColorTexIdx = mat.pbrMetallicRoughness.baseColorTexture.index;
                if (baseColorTexIdx != NPOS)
                {
                    size_t diffuseIdx = a.aTextures[baseColorTexIdx].source;
                    auto& diffuseImg = a.aImages[diffuseIdx];
                    auto diffuseImgPath = replacePathSuffix(path, diffuseImg.uri);

                    if (diffuseImgPath.ends_with(".bmp"))
                        nMesh2.meshData.materials.diffuse = Texture(diffuseImgPath, TEX_TYPE::DIFFUSE,
                                true, texMode, c);
                }
            }

            this->aM2s.push_back(nMesh2);
        }
    }
}

static void
setBuffers(std::vector<Vertex>* verts, std::vector<GLuint>* inds, MeshData* m, GLint drawMode, App* c)
{
    /* TODO: use one buffer object, or drop OBJ since gltf is here */

    std::lock_guard lock(g_mtxGlContext);

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
Model::drawGLTF(enum DRAW flags, Shader* sh, std::string_view svUniform, const m4& tmGlobal)
{
    for (auto& e : this->aM2s)
    {
        glBindVertexArray(e.meshData.vao);

        if (flags & DRAW::TEX)
        {
            e.meshData.materials.diffuse.bind(GL_TEXTURE0);
            /* TODO: implement */
            /*e.meshData.materials.normal.bind(GL_TEXTURE01);*/
        }

        m4 m = m4Iden();
        if (flags & DRAW::APPLY_TM)
        {
            m = m4Scale(m, e.vScale);
            /* TODO: other transformations */
        }

        if (sh) sh->setM4(svUniform, m * tmGlobal);

        if (e.triangleCount != NPOS)
            glDrawArrays(static_cast<GLenum>(e.mode), 0, e.triangleCount);
        else
            glDrawElements(static_cast<GLenum>(e.mode),
                           e.meshData.eboSize,
                           static_cast<GLenum>(e.indType),
                           nullptr);
    }
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
    this->size = _size;
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_UNIFORM_BUFFER, this->id);
    glBufferData(GL_UNIFORM_BUFFER, this->size, nullptr, drawMode);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void
Ubo::bindBlock(Shader* sh, std::string_view block, GLuint _point)
{
    this->point = _point;
    GLuint index = glGetUniformBlockIndex(sh->id, block.data());
    glUniformBlockBinding(sh->id, index, _point);
    LOG(OK, "uniform block: '{}' at '{}', in shader '{}'\n", block, index, sh->id);

    glBindBufferBase(GL_UNIFORM_BUFFER, _point, this->id);
    /* or */
    // glBindBufferRange(GL_UNIFORM_BUFFER, point, id, 0, size);
}

void
Ubo::bufferData(void* pData, size_t offset, size_t _size)
{
    glBindBuffer(GL_UNIFORM_BUFFER, this->id);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, _size, pData);
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
            case HASH::comment:
                p.skipWord("\n");
                break;

            case HASH::newmtl:
                p.nextWord("\n");
                ins = materials->insert({hashFNV(p.word), {}});
                break;

            case HASH::diff:
                p.nextWord("\n");
                /* TODO: implement thread pool for this kind of stuff */
                threads.emplace_back(&Texture::loadBMP,
                                     &ins.first->second.diffuse,
                                     replacePathSuffix(path, p.word),
                                     TEX_TYPE::DIFFUSE,
                                     false,
                                     texMode,
                                     c);
                break;

            case HASH::bump:
            case HASH::norm:
                p.nextWord("\n");
                threads.emplace_back(&Texture::loadBMP,
                                     &ins.first->second.normal,
                                     replacePathSuffix(path, p.word),
                                     TEX_TYPE::NORMAL,
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
