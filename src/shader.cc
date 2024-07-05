#include "shader.hh"
#include "utils.hh"

Shader::Shader(std::string_view vertexPath, std::string_view fragmentPath)
{
    loadShaders(vertexPath, fragmentPath);
}

Shader::Shader(std::string_view vertexPath, std::string_view geometryPath, std::string_view fragmentPath)
{
    loadShaders(vertexPath, geometryPath, fragmentPath);
}

void
Shader::loadShaders(std::string_view vertexPath, std::string_view fragmentPath)
{
    GLint linked;
    GLuint vertex = loadShader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = loadShader(GL_FRAGMENT_SHADER, fragmentPath);

    id = glCreateProgram();
    if (id == 0)
        LOG(FATAL, "glCreateProgram failed: {}\n", id);

    glAttachShader(id, vertex);
    glAttachShader(id, fragment);

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            glGetProgramInfoLog(id, infoLen, nullptr, infoLog.data());
            LOG(FATAL, "error linking program: {}\n", infoLog.data());
        }
        glDeleteProgram(id);
        LOG(FATAL, "error linking program.\n");
    }

#ifdef DEBUG
    glValidateProgram(id);
#endif

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void
Shader::loadShaders(std::string_view vertexPath, std::string_view geometryPath, std::string_view fragmentPath)
{
    GLint linked;
    GLuint vertex = loadShader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = loadShader(GL_FRAGMENT_SHADER, fragmentPath);
    GLuint geometry  = loadShader(GL_GEOMETRY_SHADER, geometryPath);

    id = glCreateProgram();
    if (id == 0)
        LOG(FATAL, "glCreateProgram failed: {}\n", id);

    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glAttachShader(id, geometry);

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            glGetProgramInfoLog(id, infoLen, nullptr, infoLog.data());
            LOG(FATAL, "error linking program: {}\n", infoLog.data());
        }
        glDeleteProgram(id);
        LOG(FATAL, "error linking program.\n");
    }

#ifdef DEBUG
    glValidateProgram(id);
#endif

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(geometry);
}

Shader::Shader(Shader&& other)
{
    *this = std::move(other);
}

void
Shader::operator=(Shader&& other)
{
    this->id = other.id;
    other.id = 0;
}

Shader::~Shader()
{
    if (id)
    {
        glDeleteProgram(id);
        LOG(OK, "Shader '{}' deleted\n", id);
    }
}

GLuint
Shader::loadShader(GLenum type, std::string_view path)
{
    GLuint shader;
    shader = glCreateShader(type);
    if (!shader)
        return 0;

    const auto src = loadFileToCharArray(path);
    const char* srcData = src.data();

    glShaderSource(shader, 1, &srcData, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog.data());
            CERR("error compiling shader '{}'\n{}\n", path, infoLog.data());
            throw std::runtime_error("shader compilation exception");
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void
Shader::use() const
{
    glUseProgram(id);
}

void
Shader::queryActiveUniforms()
{
    GLint maxUniformLen;
    GLint nUniforms;

    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &nUniforms);
    glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen);

    std::vector<char> uniformName(maxUniformLen, '\0');
    LOG(OK, "queryActiveUniforms for '{}':\n", this->id);

    for (int i = 0; i < nUniforms; i++)
    {
        GLint size;
        GLenum type;
        std::string_view typeName;

        glGetActiveUniform(id, i, maxUniformLen, nullptr, &size, &type, uniformName.data());
        switch (type)
        {
            case GL_FLOAT:
                typeName = "GL_FLOAT";
                break;

            case GL_FLOAT_VEC2:
                typeName = "GL_FLOAT_VEC2";
                break;

            case GL_FLOAT_VEC3:
                typeName = "GL_FLOAT_VEC3";
                break;

            case GL_FLOAT_VEC4:
                typeName = "GL_FLOAT_VEC4";
                break;

            case GL_FLOAT_MAT4:
                typeName = "GL_FLOAT_MAT4";
                break;

            case GL_FLOAT_MAT3:
                typeName = "GL_FLOAT_MAT3";
                break;

            case GL_SAMPLER_2D:
                typeName = "GL_SAMPLER_2D";
                break;

            default:
                typeName = "unknown";
                break;
        }
        LOG(OK, "\tuniformName: '{}', type: '{}'\n", uniformName.data(), typeName);
    }
}

void 
Shader::setM4(std::string_view name, const m4& m)
{
    GLint ul;
    ul = glGetUniformLocation(id, name.data());
    glUniformMatrix4fv(ul, 1, GL_FALSE, (GLfloat*)m.e);
}

void 
Shader::setM3(std::string_view name, const m3& m)
{
    GLint ul;
    ul = glGetUniformLocation(id, name.data());
    glUniformMatrix3fv(ul, 1, GL_FALSE, (GLfloat*)m.e);
}


void
Shader::setV3(std::string_view name, const v3& v)
{
    GLint ul;
    ul = glGetUniformLocation(id, name.data());
    glUniform3fv(ul, 1, (GLfloat*)v.e);
}

void
Shader::setI(std::string_view name, const GLint i)
{
    GLint ul;
    ul = glGetUniformLocation(id, name.data());
    glUniform1i(ul, i);
}

void
Shader::setF(std::string_view name, const f32 f)
{
    GLint ul;
    ul = glGetUniformLocation(id, name.data());
    glUniform1f(ul, f);
}
