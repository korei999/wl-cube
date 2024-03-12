#include "headers/shader.hh"
#include "headers/utils.hh"

#include <vector>

Shader::Shader(std::string_view vertexPath, std::string_view fragmentPath)
{
    GLint linked;
    GLuint vertex = loadShader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = loadShader(GL_FRAGMENT_SHADER, fragmentPath);

    programObject = glCreateProgram();
    if (programObject == 0)
        LOG(FATAL, "glCreateProgram failed: {}\n", programObject);

    D( glAttachShader(programObject, vertex) );
    D( glAttachShader(programObject, fragment) );

    D( glLinkProgram(programObject) );
    D( glGetProgramiv(programObject, GL_LINK_STATUS, &linked) );
    if (!linked)
    {
        GLint infoLen = 0;
        D( glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen) );
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            D( glGetProgramInfoLog(programObject, infoLen, nullptr, infoLog.data()) );
            LOG(FATAL, "error linking program: {}\n", infoLog.data());
        }
        D( glDeleteProgram(programObject) );
        LOG(FATAL, "error linking program.\n");
    }

#ifdef DEBUG
    D( glValidateProgram(programObject) );
#endif

    D( glDeleteShader(vertex) );
    D( glDeleteShader(fragment) );
}

Shader::Shader(Shader&& other)
{
    *this = std::move(other);
}

void
Shader::operator=(Shader&& other)
{
    LOG(OK, "Shader '{}' moved\n", other.programObject);

    this->programObject = other.programObject;
    other.programObject = 0;
}

Shader::~Shader()
{
    if (programObject)
    {
        D( glDeleteProgram(programObject) );
        LOG(OK, "Shader '{}' destroyed\n", programObject);
    }
}

GLuint
Shader::loadShader(GLenum type, std::string_view path)
{
    GLuint shader;

    shader = glCreateShader(type);
    D( );
    if (!shader)
        return 0;

    const auto src = loadFileToStr(path);
    const char* srcData = src.data();

    D( glShaderSource(shader, 1, &srcData, nullptr) );
    D( glCompileShader(shader) );

    GLint ok;
    D( glGetShaderiv(shader, GL_COMPILE_STATUS, &ok) );
    if (!ok)
    {
        GLint infoLen = 0;
        D( glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen) );
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            D( glGetShaderInfoLog(shader, infoLen, nullptr, infoLog.data()) );
            LOG(FATAL, "error compiling shader '{}'\n{}\n", path, infoLog.data());
        }
        D( glDeleteShader(shader) );
        return 0;
    }

    return shader;
}

void
Shader::use()
{
    D( glUseProgram(programObject) );
}

void
Shader::queryActiveUniforms()
{
    GLint maxUniformLen;
    GLint nUniforms;

    D( glGetProgramiv(programObject, GL_ACTIVE_UNIFORMS, &nUniforms) );
    D( glGetProgramiv(programObject, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen) );

    std::vector<char> uniformName(maxUniformLen, '\0');

    for (int i = 0; i < nUniforms; i++)
    {
        GLint size, location;
        GLenum type;
        std::string_view typeName;

        D( glGetActiveUniform(programObject, i, maxUniformLen, nullptr, &size, &type, uniformName.data()) );
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

            default:
                typeName = "unknown";
                break;
        }
        LOG(OK, "uniformName: '{}', type: '{}'\n", uniformName.data(), typeName);
    }
}

void 
Shader::setM4(std::string_view name, const m4& matrix)
{
    auto tU = glGetUniformLocation(programObject, name.data());
    D( );
    D( glUniformMatrix4fv(tU, 1, GL_FALSE, (GLfloat*)matrix.e););
}
