#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;

out vec4 givenColor;
out vec2 fragTexCoords;

uniform mat4 viewMatrix;

void main()
{
	gl_Position = viewMatrix * vec4(pos, 1.0f);
	givenColor = color;
}