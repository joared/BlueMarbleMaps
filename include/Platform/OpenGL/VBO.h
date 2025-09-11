#pragma once
#include <glad/glad.h>
#include <vector>
#include "Vertice.h"
struct VBO
{
	GLuint m_id;

	VBO();
	~VBO();
	
	void init(std::vector<Vertice>& vertices);
	void bind();
	void unbind();
};