#pragma once
#include <glad/glad.h>
#include <vector>
struct IBO
{
	GLuint m_id;
	IBO();
	~IBO();
	void init(std::vector<GLuint> indicies);
	void bind();
	void unbind();
};