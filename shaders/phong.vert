#version 300 es
precision mediump float;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec3 aNorm;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normMat; /* is needed for non uniform scaling */

out vec3 vsFragPos;
out vec2 vsTex;
out vec3 vsNorm;

void
main()
{
    gl_Position = proj * view * model * vec4(aPos, 1.0);
    vsFragPos = vec3(model * vec4(aPos, 1.0));
    vsTex = aTex;
    // vsNorm = aNorm;
    // vsNorm = mat3(transpose(inverse(model))) * aNorm;
    vsNorm = normMat * aNorm;
}
