#include "Texture.h"
#include <iostream>


Texture::Texture()
	:m_id(0)
{

}
Texture::~Texture()
{

}
bool Texture::init(unsigned char* data, int width, int height, GLenum format, GLenum pixelType, GLuint activeIndex)
{
	int maxNrOfTextures;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxNrOfTextures);

	if (activeIndex > maxNrOfTextures)
	{
		std::cout << "active index is larger than the max nr of texture limit; " << maxNrOfTextures << "\n";
		return false;
	}

	glGenTextures(1,&m_id);
	//TODO use glbindTextureUnit instead of shitty GL_TEXTURE enums...
	glActiveTexture(GL_TEXTURE0+activeIndex);
	glBindTexture(GL_TEXTURE_2D, m_id);
	//glBindTextureUnit(activeIndex, m_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//TODO assign the mipmap images from data read from the image as well, this could suffice as the lod cache entirely
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, pixelType, data);
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