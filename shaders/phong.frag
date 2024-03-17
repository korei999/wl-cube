#version 300 es
precision mediump float;

in vec3 vsFragPos;
in vec2 vsTex;
in vec3 vsNorm;

uniform vec3 ambLight;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform sampler2D diffuseTex;

out vec4 fragColor;

void
main()
{
    vec3 norm = normalize(vsNorm);
    vec3 lightDir = normalize(lightPos - vsFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambLight + diffuse) * texture(diffuseTex, vsTex).rgb;

    fragColor = vec4(result, 1.0);
}
