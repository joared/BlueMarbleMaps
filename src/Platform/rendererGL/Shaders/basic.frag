#version 330 core
in vec4 givenColor;
in vec2 fragTexCoords;
out vec4 FragColor;

void main()
{
	FragColor = givenColor;
}