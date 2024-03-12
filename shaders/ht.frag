#version 300 es
precision mediump float;

in vec4 position;

out vec4 fragColor;

void
main()
{
    fragColor = mix(vec4(vec3(1.0 - gl_FragCoord.z / 1.5), 1.0), position, 0.6);
    // fragColor = position;
}
