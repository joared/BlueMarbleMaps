#include "Platform/OpenGL/IBO.h"
#include <iostream>

IBO::IBO()
	:m_id(0)
{

}
IBO::~IBO()
{
	std::cout << "Deleting IBO with id: " << m_id << "\n";
	glDeleteBuffers(1, &m_id);
}
void IBO::init()
{
	glGenBuffers(1, &m_id);
}
void IBO::bufferData(std::vector<GLuint> indicies)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), &indicies[0], GL_STATIC_DRAW);
}
void IBO::allocateDynamicBuffer(GLuint size)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
}
void IBO::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}
void IBO::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}