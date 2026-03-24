#pragma once
#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#else
#include "glad/glad.h"
#endif
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