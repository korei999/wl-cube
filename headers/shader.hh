#pragma once
#include "math.hh"

#include <GLES3/gl3.h>
#include <string_view>

struct Shader
{
    GLuint programObj = 0;
    //

    Shader() = default;
    Shader(std::string_view vertShaderPath, std::string_view fragShaderPath);
    Shader(Shader&& other);
    Shader(const Shader& other) = delete;
    ~Shader();

    void operator=(Shader&& other);

    void use();
    void setM4(std::string_view name, const m4& matrix);
    void queryActiveUniforms();

private:
    GLuint loadShader(GLenum type, std::string_view path);
};
