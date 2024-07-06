#pragma once
#include "gmath.hh"
#include "gl/gl.hh"

#include <string_view>

struct Shader
{
    GLuint id = 0;

    Shader() = default;
    Shader(std::string_view vertShaderPath, std::string_view fragShaderPath);
    Shader(std::string_view vertShaderPath, std::string_view geomShaderPath, std::string_view fragShaderPath);
    Shader(Shader&& other);
    Shader(const Shader& other) = delete;
    ~Shader();

    void operator=(Shader&& other);

    void loadShaders(std::string_view vertShaderPath, std::string_view fragShaderPath);
    void loadShaders(std::string_view vertexPath, std::string_view geometryPath, std::string_view fragmentPath);
    void use() const;
    void setM3(std::string_view name, const m3& m);
    void setM4(std::string_view name, const m4& m);
    void setV3(std::string_view name, const v3& v);
    void setI(std::string_view name, const GLint i);
    void setF(std::string_view name, const f32 f);
    void queryActiveUniforms();

private:
    GLuint loadShader(GLenum type, std::string_view path);
};
