#include "headers/model.hh"
#include "headers/utils.hh"

#include <unordered_map>
#include <exception>

constexpr size_t vHash = hashFNV("v");
constexpr size_t vtHash = hashFNV("vt");
constexpr size_t vnHash = hashFNV("vn");
constexpr size_t fHash = hashFNV("f");
constexpr size_t mtllibHash = hashFNV("mtllib");
constexpr size_t oHash = hashFNV("o");
constexpr size_t usemtlHash = hashFNV("usemtl");
constexpr size_t materialHash = hashFNV("Material");
constexpr size_t sHash = hashFNV("s");
constexpr size_t offHash = hashFNV("off");
constexpr size_t newmtlHash = hashFNV("newmtl");

Model::Model(std::string_view path)
{
    LOG(OK, "loading model: '{}'...\n", path);
    loadOBJ(path);
}

Mesh
Model::parseOBJ(std::string_view path)
{
    auto file = loadFileToStr(path.data());

    std::string_view separators = " /\n";
    std::string word;
    size_t start = 0;
    size_t end = 0;

    auto isSeparator = [&](char c) -> bool
    {
        if (!file[c])
            return false;

        for (char i : separators)
            if (i == c)
                return true;

        return false;
    };

    auto nextWord = [&]()
    {
        while (file[end] && !isSeparator(file[end]))
            end++;

        word = std::string(file.begin() + start, file.begin() + end);
        start = end = end + 1;
    };

    std::vector<v3> vs;
    std::vector<v2> vts;
    std::vector<v3> vns;

    using Face = std::vector<std::array<int, 9>>;
    Face faces;

    auto wordToInt = [](const std::string& str)
    {
        if (str.size() == 0)
            return 0;
        
        return std::stoi(str);
    };

    v3 tv;
    std::array<int, 9> tf;
    while (start < file.size())
    {
        nextWord();
        size_t wordHash = hashFNV(word);
        switch (wordHash)
        {
            case vHash:
                /* get 3 floats */
                nextWord();
                tv.x = std::stof(word);
                nextWord();
                tv.y = std::stof(word);
                nextWord();
                tv.z = std::stof(word);
#ifdef MODEL
                LOG(OK, "v {} {} {}\n", tv.x, tv.y, tv.z);
#endif
                vs.push_back(tv);
                break;

            case vtHash:
                /* get 2 floats */
                nextWord();
                tv.x = std::stof(word);
                nextWord();
                tv.y = std::stof(word);
#ifdef MODEL
                LOG(OK, "vt {} {}\n", tv.x, tv.y);
#endif
                vts.push_back(tv.xy);
                break;

            case vnHash:
                /* get 3 floats */
                nextWord();
                tv.x = std::stof(word);
                nextWord();
                tv.y = std::stof(word);
                nextWord();
                tv.z = std::stof(word);
#ifdef MODEL
                LOG(OK, "vn {} {} {}\n", tv.x, tv.y, tv.z);
#endif
                vns.push_back(tv);
                break;

            case fHash:
                /* get 9 ints */
                nextWord();
                tf[0] = wordToInt(word) - 1; /* obj faces count from 1 */
                nextWord();
                tf[1] = wordToInt(word) - 1;
                nextWord();
                tf[2] = wordToInt(word) - 1;
                nextWord();
                tf[3] = wordToInt(word) - 1;
                nextWord();
                tf[4] = wordToInt(word) - 1;
                nextWord();
                tf[5] = wordToInt(word) - 1;
                nextWord();
                tf[6] = wordToInt(word) - 1;
                nextWord();
                tf[7] = wordToInt(word) - 1;
                nextWord();
                tf[8] = wordToInt(word) - 1;
#ifdef MODEL
                LOG(OK, "f {}/{}/{} {}/{}/{} {}/{}/{}\n", tf[0], tf[1], tf[2], tf[3], tf[4], tf[5], tf[6], tf[7], tf[8]);
#endif
                faces.push_back(tf);
                break;

            default:
                /* nextlines are gonna be empty tokens, skip them */
                break;
        }
    }

#ifdef MODEL
    LOG(OK, "vs: {}\tvts: {}\tvns: {}\tfaces: {}\n", vs.size(), vts.size(), vns.size(), faces.size());
#endif

    /* next we need to compose vertex buffer from faces */
    Mesh buff;
    buff.vs.reserve(vs.size());
    buff.indices.reserve(faces.size() * 3);

    if (!vts.size())
        vts.push_back({0, 0});
    if (!vns.size())
        vns.push_back({0, 0, 0});

    std::unordered_map<VertexPos, GLuint> uniqFaces;
    GLuint faceIdx = 0;

    for (auto& face : faces)
    {
        /* three vertices for each face */
        for (size_t i = 0; i < face.size(); i += 3)
        {
            VertexPos p {face[i], face[i + 1], face[i + 2]};
            if (p[1] == -1)
                p[1] = 0;

            auto insTry = uniqFaces.insert({p, faceIdx});
            if (insTry.second) /* false if we tried to insert duplicate */
            {
                /* first v3 positions, second v2 textures, last v3 normals */
                buff.vs.push_back({vs[ p[0] ], vts[ p[1] ], vns[ p[2] ]});
                buff.indices.push_back(faceIdx++);
            }
            else
            {
                buff.indices.push_back(insTry.first->second);
            }
        }
    }
    LOG(OK, "vs.size: {}\n", buff.vs.size());
    LOG(OK, "indices.size: {}\n", buff.indices.size());
#ifdef MODEL
    for (auto& i : buff.indices)
        LOG(OK, "{} {} {}\n", buff.vs[i].pos.x, buff.vs[i].pos.y, buff.vs[i].pos.z);
#endif
    buff.vs.shrink_to_fit();
    buff.indices.shrink_to_fit();

    return buff;
}

void
Model::loadOBJ(std::string_view path)
{
    if (m.vs.size())
    {
        LOG(WARNING, "model '{}' already has content.\n", (void*)this);
        return;
    }

    try
    {
        m = parseOBJ(path);
    }
    catch (const std::exception& e)
    {
        LOG(FATAL, "parseOBJ error: {}\n", e.what());
    }

    setBuffers();
}

void
Model::setBuffers()
{
	D( glGenVertexArrays(1, &vao) );
	D( glBindVertexArray(vao) );

	D( glGenBuffers(1, &vbo) );
	D( glBindBuffer(GL_ARRAY_BUFFER, vbo) );
	D( glBufferData(GL_ARRAY_BUFFER, m.vs.size() * sizeof(*m.vs.data()), m.vs.data(), GL_STATIC_DRAW) );

	D( glGenBuffers(1, &ebo) );
	D( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) );
	D( glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices.size() * sizeof(*m.indices.data()), m.indices.data(), GL_STATIC_DRAW) );

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
	/* positions */
	D( glEnableVertexAttribArray(0) );
	D( glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, sizeof(*m.vs.data()), (void*)0) );
	/* texture coords */
	D (glEnableVertexAttribArray(1) );
	D( glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, sizeof(*m.vs.data()), (void*)(sizeof(f32) * v3Size)) );
	/* normals */
	D( glEnableVertexAttribArray(2) );
	D( glVertexAttribPointer(2, v3Size, GL_FLOAT, GL_FALSE, sizeof(*m.vs.data()), (void*)(sizeof(f32) * (v3Size + v2Size))) );

	D( glBindVertexArray(0) );
}

void
Model::drawMesh()
{
	D( glBindVertexArray(vao) );
	D( glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, 0) );
}
