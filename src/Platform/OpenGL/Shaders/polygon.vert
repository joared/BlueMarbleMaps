#version 330 core

layout (location = 0) in vec3 inPos;   // from Vertex.x,y,z
layout (location = 1) in vec4 inColor; // from Vertex.r,g,b,a

uniform mat4 viewMatrix; // Model-View-Projection matrix

out vec4 vColor;

void main() 
{
    gl_Position = viewMatrix * vec4(inPos, 1.0);
    vColor = inColor;
}
