#version 300 es
precision mediump float;

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTex;
layout (location = 2) in vec3 vNorm;

layout (std140) uniform ProjView
{
    mat4 ubProj;
    mat4 ubView;
};

uniform mat4 model;

out vec4 pos;

void
main()
{
    pos = model * vec4(vPos, 1.0);
    gl_Position = ubProj * ubView * model * vec4(vPos, 1.0);
}
