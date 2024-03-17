#include "headers/shader.hh"
#include "headers/utils.hh"

Shader::Shader(std::string_view vertexPath, std::string_view fragmentPath)
{
    loadShaders(vertexPath, fragmentPath);
}

void
Shader::loadShaders(std::string_view vertexPath, std::string_view fragmentPath)
{
    GLint linked;
    GLuint vertex = loadShader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = loadShader(GL_FRAGMENT_SHADER, fragmentPath);

    progObj = glCreateProgram();
    if (progObj == 0)
        LOG(FATAL, "glCreateProgram failed: {}\n", progObj);

    D( glAttachShader(progObj, vertex) );
    D( glAttachShader(progObj, fragment) );

    D( glLinkProgram(progObj) );
    D( glGetProgramiv(progObj, GL_LINK_STATUS, &linked) );
    if (!linked)
    {
        GLint infoLen = 0;
        D( glGetProgramiv(progObj, GL_INFO_LOG_LENGTH, &infoLen) );
        if (infoLen > 1)
        {
            std::vector<char> infoLog(infoLen + 1, {});
            D( glGetProgramInfoLog(progObj, infoLen, nullptr, infoLog.data()) );
            LOG(FATAL, "error linking program: {}\n", infoLog.data());
        }
        D( glDeleteProgram(progObj) );
        LOG(FATAL, "error linking program.\n");
    }

#ifdef DEBUG
    D( glValidateProgram(progObj) );
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
    this->progObj = other.progObj;
    other.progObj = 0;
}

Shader::~Shader()
{
    if (progObj)
    {
        D( glDeleteProgram(progObj) );
        LOG(OK, "Shader '{}' destroyed\n", progObj);
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

    const auto src = fileLoad(path);
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
    D( glUseProgram(progObj) );
}

void
Shader::queryActiveUniforms()
{
    GLint maxUniformLen;
    GLint nUniforms;

    D( glGetProgramiv(progObj, GL_ACTIVE_UNIFORMS, &nUniforms) );
    D( glGetProgramiv(progObj, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen) );

    std::vector<char> uniformName(maxUniformLen, '\0');
    LOG(OK, "queryActiveUniforms for '{}':\n", this->progObj);

    for (int i = 0; i < nUniforms; i++)
    {
        GLint size;
        GLenum type;
        std::string_view typeName;

        D( glGetActiveUniform(progObj, i, maxUniformLen, nullptr, &size, &type, uniformName.data()) );
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
    auto tU = glGetUniformLocation(progObj, name.data());
    D( );
    D( glUniformMatrix4fv(tU, 1, GL_FALSE, (GLfloat*)m.e););
}

void 
Shader::setM3(std::string_view name, const m3& m)
{
    auto tU = glGetUniformLocation(progObj, name.data());
    D( );
    D( glUniformMatrix3fv(tU, 1, GL_FALSE, (GLfloat*)m.e););
}


void
Shader::setV3(std::string_view name, const v3& v)
{
    auto ul = glGetUniformLocation(progObj, name.data());
    D( );
    D( glUniform3fv(ul, 1, (GLfloat*)v.e););
}
