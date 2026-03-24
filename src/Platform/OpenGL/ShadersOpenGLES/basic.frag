#version 300 es
precision mediump float;

in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D texture0;

out vec4 FragColor;

void main()
{
    FragColor = vColor * texture(texture0, vTexCoord);
}
