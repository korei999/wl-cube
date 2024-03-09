#pragma once
#include "mmath.hh"

#include <GLES3/gl3.h>
#include <string_view>

struct Shader
{
    GLuint programObject = 0;
    //

    Shader() = default;
    Shader(std::string_view vertShaderPath, std::string_view fragShaderPath);
    Shader(Shader&& other);
    ~Shader();

    void operator=(Shader&& other);

    void use();
    void setMat4(std::string_view name, cm4 matrix);

private:
    GLuint loadShader(GLenum type, std::string_view path);
};
