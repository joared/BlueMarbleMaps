#include "IBO.h"

IBO::IBO()
	:m_id(0)
{

}
IBO::~IBO()
{
	glDeleteBuffers(1, &m_id);
}
void IBO::init(std::vector<GLuint> indicies)
{
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies),indicies.data(),GL_STATIC_DRAW);
}
void IBO::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}
void IBO::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}