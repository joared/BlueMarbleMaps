#include "VBO.h"

VBO::VBO()
	:m_id(0)
{

}
VBO::~VBO()
{

}

void VBO::init(std::vector<Vertice>& vertices)
{
	glGenBuffers(1, &m_id);
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertice) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
}
void VBO::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}
void VBO::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}