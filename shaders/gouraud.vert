#version 300 es
precision mediump float;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec3 aNorm;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normMat; /* is needed for non uniform scaling */

uniform vec3 ambLight;
uniform vec3 lightPos;
uniform vec3 lightColor;

out vec2 vsTex;
out vec4 vsGouraudColor;

vec4
gouraud()
{
    vec3 norm = normalize(normMat * aNorm);
    vec3 vsFragPos = vec3(model * vec4(aPos, 1.0));

    vec3 lightDir = normalize(lightPos - vsFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambLight + diffuse);

    return vec4(result, 1.0);
}

void
main()
{
    gl_Position = proj * view * model * vec4(aPos, 1.0);
    vsGouraudColor = gouraud();
    vsTex = aTex;
}
