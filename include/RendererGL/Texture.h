#pragma once
#include <glad/glad.h>
#include <string>
struct Texture
{
	GLuint m_id;

	Texture();
	~Texture();
	bool init(unsigned char* data, int width, int height, int format, GLenum pixelType, GLuint activeIndex);
	void bind();
	void unbind();
};