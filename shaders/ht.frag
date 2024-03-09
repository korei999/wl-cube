#version 300 es
precision mediump float;

in vec4 position;

out vec4 fragColor;

void
main()
{
    fragColor = position;
}
