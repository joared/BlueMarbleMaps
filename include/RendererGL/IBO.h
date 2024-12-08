#pragma once
#include <glad/glad.h>
struct IBO
{
	GLuint m_id;
	IBO();
	~IBO();
	void init();
	void bind();
	void unbind();
};