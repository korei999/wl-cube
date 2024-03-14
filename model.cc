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

void
Model::parseOBJ(std::string_view path)
{
    auto file = loadFileToStr(path.data());

    std::string_view seps = " /\n";
    std::string word;
    size_t start = 0;
    size_t end = 0;

    auto isSeparator = [&](char c, std::string_view separotors) -> bool
    {
        if (!file[c])
            return false;

        for (char i : separotors)
            if (i == c)
                return true;

        return false;
    };

    auto nextWord = [&](std::string_view separotors)
    {
        while (file[end] && !isSeparator(file[end], separotors))
            end++;

        word = std::string(file.begin() + start, file.begin() + end);
        start = end = end + 1;
    };

    std::vector<v3> vs;
    std::vector<v2> vts;
    std::vector<v3> vns;

    using Face = std::vector<std::array<int, 9>>;
    std::vector<Face> objects;

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
        nextWord(seps);
        size_t wordHash = hashFNV(word);
        switch (wordHash)
        {
            case oHash:
                objects.push_back({});
                break;

            case vHash:
                /* get 3 floats */
                nextWord(seps);
                tv.x = std::stof(word);
                nextWord(seps);
                tv.y = std::stof(word);
                nextWord(seps);
                tv.z = std::stof(word);
#ifdef MODEL
                LOG(OK, "v {} {} {}\n", tv.x, tv.y, tv.z);
#endif
                vs.push_back(tv);
                break;

            case vtHash:
                /* get 2 floats */
                nextWord(seps);
                tv.x = std::stof(word);
                nextWord(seps);
                tv.y = std::stof(word);
#ifdef MODEL
                LOG(OK, "vt {} {}\n", tv.x, tv.y);
#endif
                vts.push_back(tv.xy);
                break;

            case vnHash:
                /* get 3 floats */
                nextWord(seps);
                tv.x = std::stof(word);
                nextWord(seps);
                tv.y = std::stof(word);
                nextWord(seps);
                tv.z = std::stof(word);
#ifdef MODEL
                LOG(OK, "vn {} {} {}\n", tv.x, tv.y, tv.z);
#endif
                vns.push_back(tv);
                break;

            case fHash:
                /* get 9 ints */
                nextWord(seps);
                tf[0] = wordToInt(word) - 1; /* obj faces count from 1 */
                nextWord(seps);
                tf[1] = wordToInt(word) - 1;
                nextWord(seps);
                tf[2] = wordToInt(word) - 1;
                nextWord(seps);
                tf[3] = wordToInt(word) - 1;
                nextWord(seps);
                tf[4] = wordToInt(word) - 1;
                nextWord(seps);
                tf[5] = wordToInt(word) - 1;
                nextWord(seps);
                tf[6] = wordToInt(word) - 1;
                nextWord(seps);
                tf[7] = wordToInt(word) - 1;
                nextWord(seps);
                tf[8] = wordToInt(word) - 1;
#ifdef MODEL
                LOG(OK, "f {}/{}/{} {}/{}/{} {}/{}/{}\n", tf[0], tf[1], tf[2], tf[3], tf[4], tf[5], tf[6], tf[7], tf[8]);
#endif
                objects.back().push_back(tf);
                break;

            default:
                /* nextlines are gonna be empty tokens, skip them */
                break;
        }
    }

    LOG(OK, "vs: {}\tvts: {}\tvns: {}\tobjects: {}\n", vs.size(), vts.size(), vns.size(), objects.size());

    /* TODO: this still might be usefull */
    // buff.vs.reserve(vs.size());
    // buff.indices.reserve(faces.size() * 3);

    // if (!vts.size())
        // vts.push_back({0, 0});
    // if (!vns.size())
        // vns.push_back({0, 0, 0});

    // for (auto& face : faces)
    // {
        // /* three vertices for each face */
        // for (size_t i = 0; i < face.size(); i += 3)
        // {
            // VertexPos p {face[i], face[i + 1], face[i + 2]};
            // if (p[1] == -1)
                // p[1] = 0;

            // auto insTry = uniqFaces.insert({p, faceIdx});
            // if (insTry.second) /* false if we tried to insert duplicate */
            // {
                // /* first v3 positions, second v2 textures, last v3 normals */
                // buff.vs.push_back({vs[ p[0] ], vts[ p[1] ], vns[ p[2] ]});
                // buff.indices.push_back(faceIdx++);
            // }
            // else
            // {
                // buff.indices.push_back(insTry.first->second);
            // }
        // }
    // }

    if (!vts.size())
        vts.push_back({});
    if (!vns.size())
        vns.push_back({});

    std::unordered_map<VertexPos, GLuint> uniqFaces;

    for (auto& faces : objects)
    {
        Mesh buff;
        GLuint faceIdx = 0;
        buff.vs.reserve(vs.size());
        buff.indices.reserve(faces.size() * 3);

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
        buff.vs.shrink_to_fit();
        buff.indices.shrink_to_fit();

        this->meshes.push_back(std::move(buff));
        setBuffers(meshes.back());
    }
    this->meshes.shrink_to_fit();
}

void
Model::loadOBJ(std::string_view path)
{
    try
    {
        parseOBJ(path);
    }
    catch (const std::exception& e)
    {
        LOG(FATAL, "parseOBJ error: {}\n", e.what());
    }
}

void
Model::setBuffers(Mesh& mesh)
{
	D( glGenVertexArrays(1, &mesh.vao) );
	D( glBindVertexArray(mesh.vao) );

	D( glGenBuffers(1, &mesh.vbo) );
	D( glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo) );
	D( glBufferData(GL_ARRAY_BUFFER, mesh.vs.size() * sizeof(*mesh.vs.data()), mesh.vs.data(), GL_STATIC_DRAW) );

	D( glGenBuffers(1, &mesh.ebo) );
	D( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo) );
	D( glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(*mesh.indices.data()), mesh.indices.data(), GL_STATIC_DRAW) );

    constexpr size_t v3Size = sizeof(v3) / sizeof(f32);
    constexpr size_t v2Size = sizeof(v2) / sizeof(f32);
    auto data = mesh.vs.data();
	/* positions */
	D( glEnableVertexAttribArray(0) );
	D( glVertexAttribPointer(0, v3Size, GL_FLOAT, GL_FALSE, sizeof(*data), (void*)0) );
	/* texture coords */
	D (glEnableVertexAttribArray(1) );
	D( glVertexAttribPointer(1, v2Size, GL_FLOAT, GL_FALSE, sizeof(*data), (void*)(sizeof(f32) * v3Size)) );
	/* normals */
	D( glEnableVertexAttribArray(2) );
	D( glVertexAttribPointer(2, v3Size, GL_FLOAT, GL_FALSE, sizeof(*data), (void*)(sizeof(f32) * (v3Size + v2Size))) );

	D( glBindVertexArray(0) );
}

void
Model::draw()
{
    for (auto& mesh : this->meshes)
    {
        D( glBindVertexArray(mesh.vao) );
        D( glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0) );
    }
}

void
Model::draw(const Mesh& mesh)
{
    D( glBindVertexArray(mesh.vao) );
    D( glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0) );
}

void
Model::draw(size_t i)
{
    D( glBindVertexArray(meshes[i].vao) );
    D( glDrawElements(GL_TRIANGLES, meshes[i].indices.size(), GL_UNSIGNED_INT, 0) );
}
