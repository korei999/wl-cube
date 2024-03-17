#version 300 es
precision mediump float;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec3 aNorm;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normMat; /* is needed for non uniform scaling */
uniform vec3 viewPos;

uniform vec3 ambLight;
uniform vec3 lightPos;
uniform vec3 lightColor;

out vec2 vsTex;
out vec4 vsGouraudColor;

vec4
gouraud()
{
    float specFactor = 0.5;
    vec3 norm = normalize(normMat * aNorm);
    vec3 fragPos = vec3(model * vec4(aPos, 1.0));

    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specFactor * spec * lightColor;

    vec3 result = (ambLight + diffuse + specular);

    return vec4(result, 1.0);
}

void
main()
{
    gl_Position = proj * view * model * vec4(aPos, 1.0);
    vsGouraudColor = gouraud();
    vsTex = aTex;
}
