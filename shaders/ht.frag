#version 300 es
precision mediump float;

in vec4 pos;

out vec4 fragColor;

void
main()
{
    // fragColor = mix(vec4(vec3(1.0 - gl_FragCoord.z), 1.0), pos, 0.05);
    fragColor = pos * 2.0f;
}
