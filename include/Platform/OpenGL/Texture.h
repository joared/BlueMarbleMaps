#pragma once
#include <glad/glad.h>
#include <memory>
#include <string>
struct Texture
{
	GLuint m_id;

	Texture();
	~Texture();
	bool init(const unsigned char* data, int width, int height, int format, GLenum pixelType, GLuint activeIndex);
	void bind();
	void unbind();
};
typedef std::shared_ptr<Texture> TexturePtr;