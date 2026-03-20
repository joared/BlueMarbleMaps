#pragma once
#if defined(__EMSCRIPTEN__)
#include <GLES2/gl2.h>
#else
#include "glad/glad.h"
#endif
#include "Platform/OpenGL/VBO.h"
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