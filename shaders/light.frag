#version 300 es
precision mediump float;

in vec4 outColor;
in vec2 outTexCoords;

uniform sampler2D diffuseTex;

out vec4 fragColor;

void
main()
{
    fragColor = vec4(texture(diffuseTex, outTexCoords).rgb, 1.0);
}
