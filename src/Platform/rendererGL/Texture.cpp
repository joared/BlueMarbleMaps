#include "Texture.h"
#include <iostream>


Texture::Texture()
	:m_id(0)
{

}
Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}
bool Texture::init(unsigned char* data, int width, int height, int format, GLenum pixelType, GLuint activeIndex)
{
	int maxNrOfTextures;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxNrOfTextures);

	if (activeIndex > maxNrOfTextures)
	{
		std::cout << "active index is larger than the max nr of texture limit; " << maxNrOfTextures << "\n";
		return false;
	}
	GLenum glFormat;
	switch (format)
	{
	case 1: glFormat = GL_R;    break;
	case 2: glFormat = GL_RG;   break;
	case 3: glFormat = GL_RGB;  break;
	case 4: glFormat = GL_RGBA; break;
	default:glFormat = GL_RGBA;
	}

	glGenTextures(1,&m_id);

	glBindTexture(GL_TEXTURE_2D, m_id);
	glBindTextureUnit(activeIndex, m_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//TODO assign the mipmap images from data read from the image as well, this could suffice as the lod cache entirely
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, GL_RGBA, pixelType, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}
void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, m_id);
}
void Texture::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}