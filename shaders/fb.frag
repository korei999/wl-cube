#version 300 es
precision mediump float;

in vec2 vsTex;

uniform sampler2D tex0;

out vec4 fragColor;

vec4 processKernel(float kernel[9]);

const float offset = 1.0 / 300.0;

vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), /* top-left */
    vec2(0.0,      offset), /* top-center */
    vec2(offset,   offset), /* top-right */
    vec2(-offset,     0.0), /* center-left */
    vec2(0.0,         0.0), /* center-center */
    vec2(offset,      0.0), /* center-right */
    vec2(-offset, -offset), /* bottom-left */
    vec2(0.0,     -offset), /* bottom-center */
    vec2(offset,  -offset)  /* bottom-right */
);

vec4
sharpen()
{
    float kernel[9] = float[](
        -1.0, -1.0, -1.0,
        -1.0,  9.0, -1.0,
        -1.0, -1.0, -1.0
    );

    return processKernel(kernel);
}

vec4
blur()
{
    float kernel[9] = float[](
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
        2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
    );

    return processKernel(kernel);
}

vec4
edge()
{
    float kernel[9] = float[](
        1.0,  1.0,  1.0,
        1.0, -8.0,  1.0,
        1.0,  1.0,  1.0
    );

    return processKernel(kernel);
}

vec4
processKernel(float kernel[9])
{
    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
        sampleTex[i] = vec3(texture(tex0, vsTex.st + offsets[i]));

    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    return vec4(col, 1.0);
}

void
main()
{
    fragColor = edge();
}
