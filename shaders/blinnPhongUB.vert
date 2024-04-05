#version 300 es
precision mediump float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

layout (std140) uniform ProjView
{
    mat4 ubProj;
    mat4 ubView;
};

uniform mat4 model;

out vec3 vsPos;
out vec2 vsTex;
out vec3 vsNorm;
out mat4 vsModel;

void
main()
{
    gl_Position = ubProj * ubView * model * vec4(aPos, 1.0);

    vsPos = aPos;
    vsTex = aTex;
    vsNorm = aNorm;
    vsModel = model;
}
