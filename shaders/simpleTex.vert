#version 300 es
precision mediump float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

layout (std140) uniform ubProjView
{
    mat4 uProj;
    mat4 uView;
};

uniform mat4 uModel;

out vec2 vsTex;

void
main()
{
    vsTex = aTex;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
