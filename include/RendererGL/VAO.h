#pragma once
#include <glad/glad.h>
#include "VBO.h"
struct VAO
{
	GLuint m_id;

	VAO();
	~VAO();
	void init();
	void bind();
	void link(VBO& vbo, GLuint layout, GLuint nrOfComponents, GLenum type, GLsizeiptr stride, GLvoid* offset);
	void linkInt(VBO& vbo, GLuint layout, GLuint nrOfComponents, GLenum type, GLsizeiptr stride, GLvoid* offset);
	void unbind();
};