#version 300 es
precision mediump float;

in vec4 pos;

uniform vec3 lightColor;

out vec4 fragColor;

void
main()
{
    fragColor = vec4(lightColor, 1.0);
}
