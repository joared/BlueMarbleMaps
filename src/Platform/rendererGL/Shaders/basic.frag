#version 330 core
in vec4 givenColor;
in vec2 fragTexCoords;
out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
	FragColor = givenColor * texture(texture0, fragTexCoords);
}