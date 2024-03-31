#pragma once
#include "gmath.hh"

#include <GLES3/gl3.h>
#include <string_view>

struct Shader
{
    GLuint id = 0;

    Shader() = default;
    Shader(std::string_view vertShaderPath, std::string_view fragShaderPath);
    Shader(Shader&& other);
    Shader(const Shader& other) = delete;
    ~Shader();

    void operator=(Shader&& other);

    void loadShaders(std::string_view vertShaderPath, std::string_view fragShaderPath);
    void use();
    void setM3(std::string_view name, const m3& m);
    void setM4(std::string_view name, const m4& m);
    void setV3(std::string_view name, const v3& v);
    void setF(std::string_view name, cf32 f);
    void queryActiveUniforms();

private:
    GLuint loadShader(GLenum type, std::string_view path);
};
