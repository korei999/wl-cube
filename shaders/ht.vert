#version 300 es

layout(location = 0) in vec4 vPos;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec4 pos;

void
main()
{
    pos= model * vPos;
    gl_Position = proj * view * model * vPos;
}
