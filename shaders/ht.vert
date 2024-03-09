#version 300 es

layout(location = 0) in vec4 vPosition;

out vec4 position;

uniform mat4 transform;

void
main()
{
    vec4 nt = transform * vPosition;
    position = nt;
    gl_Position = nt;
}
