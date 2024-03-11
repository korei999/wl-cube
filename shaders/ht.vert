#version 300 es

layout(location = 0) in vec4 vPosition;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec4 position;

void
main()
{
    position = vPosition;
    gl_Position = proj * view * model * vPosition;
}
