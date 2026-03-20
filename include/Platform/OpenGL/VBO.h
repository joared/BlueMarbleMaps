#pragma once
#if defined(__EMSCRIPTEN__)
#include <GLES2/gl2.h>
#else
#include "glad/glad.h"
#endif
#include <vector>
#include "Platform/OpenGL/Vertice.h"
struct VBO
{
	GLuint m_id;
	GLuint m_vertexCount;
	VBO();
	~VBO();
	
	void init();
	void bufferData(std::vector<Vertice>& vertices);
	void allocateDynamicBuffer(GLuint size);
	void bind();
	void unbind();
};