#version 300 es
precision mediump float;

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTex;
layout (location = 2) in vec3 vNorm;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec4 pos;

void
main()
{
    pos = model * vec4(vPos, 1.0);
    gl_Position = proj * view * model * vec4(vPos, 1.0);
}
