#pragma once
#include "math.hh"

#include <GLES3/gl3.h>
#include <string_view>

struct Shader
{
    GLuint programObject = 0;
    //

    Shader() = default;
    Shader(std::string_view vertShaderPath, std::string_view fragShaderPath);
    Shader(Shader&& other);
    Shader(Shader& other) = delete;
    ~Shader();

    void operator=(Shader&& other);

    void use();
    void setMat4(std::string_view name, const m4& matrix);

private:
    GLuint loadShader(GLenum type, std::string_view path);
};
