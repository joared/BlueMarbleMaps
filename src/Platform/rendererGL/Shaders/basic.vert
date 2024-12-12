#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoords;

out vec4 givenColor;
out vec2 fragTexCoords;

void main()
{
	gl_Position = vec4(pos, 1.0f);
	givenColor = color;
	fragTexCoords = texCoords;
}