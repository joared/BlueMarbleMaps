#version 330 core
out vec4 FragColor;

uniform sampler2D texture0;

in DATA
{
	vec4 position;
	vec4 color;
	vec2 texCoord;
}frag_in;

void main()
{
	FragColor = frag_in.color * texture(texture0, frag_in.texCoord);
}