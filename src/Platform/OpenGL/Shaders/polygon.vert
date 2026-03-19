#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;

uniform mat4 viewMatrix;

out DATA
{
    vec4 position;
    vec4 color;
    vec2 texCoord;
}vert_out;

void main()
{
    vec4 mPos = viewMatrix * vec4(pos, 1.0f);
    gl_Position = mPos;
    vert_out.position = mPos;
    vert_out.color = color;
}
