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
    GLuint vertex = this->loadShader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = this->loadShader(GL_FRAGMENT_SHADER, fragmentPath);

    this->id = glCreateProgram();
    if (this->id == 0)
        LOG(FATAL, "glCreateProgram failed: {}\n", this->id);

    glAttachShader(this->id, vertex);
    glAttachShader(this->id, fragment);

    glLinkProgram(this->id);
    glGetProgramiv(this->id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &infoLen);
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
    glValidateProgram(this->id);
#endif

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void
Shader::loadShaders(std::string_view vertexPath, std::string_view geometryPath, std::string_view fragmentPath)
{
    GLint linked;
    GLuint vertex = this->loadShader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = this->loadShader(GL_FRAGMENT_SHADER, fragmentPath);
    GLuint geometry = this->loadShader(GL_GEOMETRY_SHADER, geometryPath);

    this->id = glCreateProgram();
    if (this->id == 0)
        LOG(FATAL, "glCreateProgram failed: {}\n", this->id);

    glAttachShader(this->id, vertex);
    glAttachShader(this->id, fragment);
    glAttachShader(this->id, geometry);

    glLinkProgram(this->id);
    glGetProgramiv(this->id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(this->id, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            glGetProgramInfoLog(this->id, infoLen, nullptr, infoLog.data());
            LOG(FATAL, "error linking program: {}\n", infoLog.data());
        }
        glDeleteProgram(this->id);
        LOG(FATAL, "error linking program.\n");
    }

#ifdef DEBUG
    glValidateProgram(this->id);
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
    if (this->id)
    {
        glDeleteProgram(this->id);
        /*LOG(OK, "Shader '{}' deleted\n", this->id);*/
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
    glUseProgram(this->id);
}

void
Shader::queryActiveUniforms()
{
    GLint maxUniformLen;
    GLint nUniforms;

    glGetProgramiv(this->id, GL_ACTIVE_UNIFORMS, &nUniforms);
    glGetProgramiv(this->id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen);

    std::vector<char> uniformName(maxUniformLen, '\0');
    LOG(OK, "queryActiveUniforms for '{}':\n", this->id);

    for (int i = 0; i < nUniforms; i++)
    {
        GLint size;
        GLenum type;
        std::string_view typeName;

        glGetActiveUniform(this->id, i, maxUniformLen, nullptr, &size, &type, uniformName.data());
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
    ul = glGetUniformLocation(this->id, name.data());
    glUniformMatrix4fv(ul, 1, GL_FALSE, (GLfloat*)m.e);
}

void 
Shader::setM3(std::string_view name, const m3& m)
{
    GLint ul;
    ul = glGetUniformLocation(this->id, name.data());
    glUniformMatrix3fv(ul, 1, GL_FALSE, (GLfloat*)m.e);
}


void
Shader::setV3(std::string_view name, const v3& v)
{
    GLint ul;
    ul = glGetUniformLocation(this->id, name.data());
    glUniform3fv(ul, 1, (GLfloat*)v.e);
}

void
Shader::setI(std::string_view name, const GLint i)
{
    GLint ul;
    ul = glGetUniformLocation(this->id, name.data());
    glUniform1i(ul, i);
}

void
Shader::setF(std::string_view name, const f32 f)
{
    GLint ul;
    ul = glGetUniformLocation(this->id, name.data());
    glUniform1f(ul, f);
}
