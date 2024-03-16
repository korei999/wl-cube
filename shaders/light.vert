#version 300 es

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTex;
layout(location = 2) in vec3 vNorm;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;

out vec2 outTexCoords;

void
main()
{
    outTexCoords = vTex;
    gl_Position = proj * view * model * vec4(vPos, 1.0);
}
