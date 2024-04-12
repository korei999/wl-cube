#include "headers/model.hh"
#include "headers/utils.hh"

#include <thread>
#include <unordered_map>
#include <exception>

static void parseMtl(std::unordered_map<u64, Texture>* materials, std::string_view path, WlClient* c);

constexpr size_t vHash = hashFNV("v");
constexpr size_t vtHash = hashFNV("vt");
constexpr size_t vnHash = hashFNV("vn");
constexpr size_t fHash = hashFNV("f");
constexpr size_t mtllibHash = hashFNV("mtllib");
constexpr size_t oHash = hashFNV("o");
constexpr size_t usemtlHash = hashFNV("usemtl");
constexpr size_t commentHash = hashFNV("#");

Model::Model(Model&& other)
{
    this->meshes = std::move(other.meshes);
    this->objects = std::move(other.objects);
}

Model::Model(std::string_view path)
{
    loadOBJ(path);
}

Model::~Model()
{
    if (this->meshes.size())
    {
        for (auto& mesh : this->meshes)
        {
            glDeleteVertexArrays(1, &mesh.vao);
            glDeleteBuffers(1, &mesh.vbo);
            glDeleteBuffers(1, &mesh.ebo);
        }
    }

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
    this->meshes = std::move(other.meshes);
    return *this;
}

void
Model::parseOBJ(std::string_view path, GLint drawMode, WlClient* c)
{
    Parser objP(path, " /\n");

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
        // std::vector<FaceData> fs; /* face data indices */
        std::vector<MaterialData> fss;
        std::string o; /* object name */
        std::string usemtl; /* material name */
    };

    std::vector<Object> objects;

    auto wordToInt = [](const std::string& str)
    {
        if (str.size() == 0)
            return 0;
        
        return std::stoi(str);
    };

    while (!objP.finished())
    {
        objP.nextWord();

        size_t wordHash = hashFNV(objP.word);
        v3 tv;
        FaceData tf;

        switch (wordHash)
        {
            case commentHash:
                objP.skipWord("\n");
                break;

            case mtllibHash:
                objP.nextWord("\n");
                mtllibName = objP.word;
                break;

            case usemtlHash:
                objP.nextWord("\n");
                /* TODO: figure out how to split 1 object that has differrent materials */
                if (objects.back().usemtl.empty())
                    objects.back().usemtl = objP.word;

                objects.back().fss.push_back({});
                objects.back().fss.back().usemtl = objP.word;
                break;

            case oHash:
                /* give space for new object */
                objects.push_back({});
                objP.nextWord("\n");

                objects.back().o = objP.word;
                break;

            case vHash:
                /* get 3 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);
                objP.nextWord();
                tv.z = std::stof(objP.word);

                vs.push_back(tv);
                break;

            case vtHash:
                /* get 2 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);

                vts.push_back(tv.xy);
                break;

            case vnHash:
                /* get 3 floats */
                objP.nextWord();
                tv.x = std::stof(objP.word);
                objP.nextWord();
                tv.y = std::stof(objP.word);
                objP.nextWord();
                tv.z = std::stof(objP.word);

                vns.push_back(tv);
                break;

            case fHash:
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
                // objects.back().fs.push_back(tf);
                objects.back().fss.back().fs.push_back(tf);
                break;

            default:
                /* nextlines are empty tokens, skip them */
                break;
        }
    }
    LOG(OK, "vs: {}\tvts: {}\tvns: {}\tobjects: {}\n", vs.size(), vts.size(), vns.size(), objects.size());
#ifdef Model
    for (auto& i : objects)
        LOG(OK, "o: '{}', usemtl: '{}'\n", i.o, i.usemtl);
#endif

    /* parse mtl file and load all the textures, later move them to the models */
    std::unordered_map<u64, Texture> materialsMap(objects.size() * 2);

    if (!mtllibName.empty())
    {
        std::string pathToMtl = replaceFileSuffixInPath(path, mtllibName);
        parseMtl(&materialsMap, pathToMtl, c);
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

        for (auto& faces : materials.fss)
        {
            Mesh mesh {
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

                    auto insTry = uniqFaces.insert({p, faceIdx});
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
            }
            setBuffers(verts, inds, mesh, drawMode, c);
            mesh.eboSize = inds.size();

            auto foundTex = materialsMap.find(hashFNV(faces.usemtl));

            this->objects.back().push_back(std::move(mesh));
            this->objects.back().back().diffuse = std::move(foundTex->second);

            /* do not forget to clear buffers */
            verts.clear();
            inds.clear();
        }
    }

    this->objects.shrink_to_fit();
}

void
Model::loadOBJ(std::string_view path, GLint drawMode, WlClient* c)
{
    try
    {
        LOG(OK, "loading model: '{}'...\n", path);
        this->parseOBJ(path, drawMode, c);
        this->savedPath = path;
    }
    catch (const std::exception& e)
    {
        LOG(FATAL, "parseOBJ error: {}\n", e.what());
    }
}

void
Model::setBuffers(std::vector<Vertex>& verts, std::vector<GLuint>& inds, Mesh& m, GLint drawMode, WlClient* c)
{
    std::lock_guard lock(glContextMtx);

    c->bindGlContext();

    auto vsData = verts.data();
    auto inData = inds.data();

    glGenVertexArrays(1, &m.vao);
    glBindVertexArray(m.vao);

    glGenBuffers(1, &m.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(*vsData), vsData, drawMode);

    glGenBuffers(1, &m.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(*inData), inData, drawMode);

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
    /* positions */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, sizeof(*vsData), (void*)0);
    /* texture coords */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, sizeof(*vsData), (void*)(sizeof(f32) * v3Size));
    /* normals */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, v3Size, GL_FLOAT, GL_FALSE, sizeof(*vsData), (void*)(sizeof(f32) * (v3Size + v2Size)));

    glBindVertexArray(0);

    c->unbindGlContext();
}

void
Model::draw()
{
    for (auto& mesh : meshes)
    {
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr);
    }
}

void
Model::draw(size_t i)
{
    glBindVertexArray(meshes[i].vao);
    glDrawElements(GL_TRIANGLES, meshes[i].eboSize, GL_UNSIGNED_INT, nullptr);
}

void
Model::draw(const Mesh& mesh)
{
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr);
}


void
Model::drawInstanced(GLsizei count)
{
    for (auto& mesh : meshes)
    {
        glBindVertexArray(mesh.vao);
        glDrawElementsInstanced(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr, count);
    }
}

void
Model::drawInstanced(size_t i, GLsizei count)
{
    glBindVertexArray(meshes[i].vao);
    glDrawElementsInstanced(GL_TRIANGLES, meshes[i].eboSize, GL_UNSIGNED_INT, nullptr, count);
}

void
Model::drawInstanced(const Mesh& mesh, GLsizei count)
{
    glBindVertexArray(mesh.vao);
    glDrawElementsInstanced(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr, count);
}

void
Model::drawTex()
{
    for (auto& materials : objects)
        for (auto& mesh : materials)
        {
            mesh.diffuse.bind(GL_TEXTURE0);
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, mesh.eboSize, GL_UNSIGNED_INT, nullptr);
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
    q.meshes.resize(1, {});

    glGenVertexArrays(1, &q.meshes[0].vao);
    glBindVertexArray(q.meshes[0].vao);

    glGenBuffers(1, &q.meshes[0].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q.meshes[0].vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, drawMode);

    glGenBuffers(1, &q.meshes[0].ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q.meshes[0].ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, drawMode);
    q.meshes[0].eboSize = LEN(quadIndices);

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

    LOG(OK, "quad '{}' created\n", q.meshes[0].vao);
    q.savedPath = "Quad";
    return q;
}

Model
getPlane(GLint drawMode)
{
    f32 planeVertices[] {
        // positions            // texcoords   // normals         
         25.0f, -0.5f,  25.0f,  25.0f,  0.0f,  0.0f, 1.0f, 0.0f,  
        -25.0f, -0.5f, -25.0f,   0.0f, 25.0f,  0.0f, 1.0f, 0.0f,  
        -25.0f, -0.5f,  25.0f,   0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  
                                                                  
        -25.0f, -0.5f, -25.0f,   0.0f, 25.0f,  0.0f, 1.0f, 0.0f,  
         25.0f, -0.5f,  25.0f,  25.0f,  0.0f,  0.0f, 1.0f, 0.0f,  
         25.0f, -0.5f, -25.0f,  25.0f, 25.0f,  0.0f, 1.0f, 0.0f 
    };

    Model q;
    q.meshes.resize(1, {});

    glGenVertexArrays(1, &q.meshes[0].vao);
    glBindVertexArray(q.meshes[0].vao);

    glGenBuffers(1, &q.meshes[0].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q.meshes[0].vbo);
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

    LOG(OK, "plane '{}' created\n", q.meshes[0].vao);
    return q;
}

void
drawQuad(const Model& q)
{
    glBindVertexArray(q.meshes[0].vao);
    glDrawElements(GL_TRIANGLES, q.meshes[0].eboSize, GL_UNSIGNED_INT, nullptr);
}

void
drawPlane(const Model& q)
{
    glBindVertexArray(q.meshes[0].vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Model
getCube(GLint drawMode)
{
    float cubeVertices[] {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
    };

    Model q;
    q.meshes.resize(1, {});

    glGenVertexArrays(1, &q.meshes[0].vao);
    glBindVertexArray(q.meshes[0].vao);

    glGenBuffers(1, &q.meshes[0].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, q.meshes[0].vbo);
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

    LOG(OK, "cube '{}' created\n", q.meshes[0].vao);
    return q;
}

void
drawCube(const Model& q)
{
    glBindVertexArray(q.meshes[0].vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void
parseMtl(std::unordered_map<u64, Texture>* materials, std::string_view path, WlClient* c)
{
    constexpr size_t newmtlHash = hashFNV("newmtl");
    constexpr size_t mapKdHash = hashFNV("map_Kd"); /* diffuse texture */

    Parser p(path, " \n");
    u64 diffuseTexHash = 0;
    auto ins = materials->insert({u64(0), {}}); /* it's 'impossible' to get this type otherwise */
    materials->clear();

    std::vector<std::thread> threads;

    while (!p.finished())
    {
        p.nextWord();

        size_t wordHash = hashFNV(p.word);

        switch (wordHash)
        {
            case commentHash:
                p.skipWord("\n");
                break;

            case newmtlHash:
                p.nextWord("\n");
                ins = materials->insert({hashFNV(p.word), {}});
                break;

            case mapKdHash:
                p.nextWord("\n");
                /* TODO: implement thread pool for this kind of stuff */
                threads.emplace_back(&Texture::loadBMP,
                                     &ins.first->second,
                                     replaceFileSuffixInPath(path, p.word),
                                     false,
                                     GL_MIRRORED_REPEAT,
                                     c);
                break;

            default:
                break;
        }
    }

    for (auto& thread : threads)
        thread.join();
}
