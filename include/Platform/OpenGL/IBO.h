#pragma once
#include <glad/glad.h>
#include <vector>
struct IBO
{
	GLuint m_id;
	IBO();
	~IBO();
	void init();
	void bufferData(std::vector<GLuint> indicies);
	void allocateDynamicBuffer(GLuint size);
	void bind();
	void unbind();
};