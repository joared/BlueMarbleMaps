#version 300 es

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoords;

uniform mat4 viewMatrix;

out vec4 vColor;

void main()
{
    vec4 mPos = viewMatrix * vec4(pos, 1.0);
    gl_Position = mPos;

    vColor = color;
}
