#version 300 es

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTex;
layout(location = 2) in vec3 vNorm;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;

out vec4 outColor;
out vec2 outTexCoords;

void
main()
{
    float dist = distance(lightPos, vPos);
    vec3 dv = vec3(dist, dist, dist);
    outColor = mix(vec4(dv, 1.0), vec4(lightPos, 1.0), 0.8);
    outTexCoords = vTex;
    gl_Position = proj * view * model * vec4(vPos, 1.0);
}
