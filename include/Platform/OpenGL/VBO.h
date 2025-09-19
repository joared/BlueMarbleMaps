#pragma once
#include <glad/glad.h>
#include <vector>
#include "Platform/OpenGL/Vertice.h"
struct VBO
{
	GLuint m_id;
	GLuint m_vertexCount;
	VBO();
	~VBO();
	
	void init(std::vector<Vertice>& vertices);
	void bind();
	void unbind();
};