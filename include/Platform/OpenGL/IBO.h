#pragma once
#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#else
#include "glad/glad.h"
#endif
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