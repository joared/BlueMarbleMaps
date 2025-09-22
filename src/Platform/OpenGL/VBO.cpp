#include "Platform/OpenGL/VBO.h"
#include <iostream>

VBO::VBO()
	:m_id(0),
	 m_vertexCount(0)
{

}
VBO::~VBO()
{
	std::cout << "Deleting VAO with id: " << m_id << "\n";
	glDeleteBuffers(1, &m_id);
}

void VBO::init()
{
	glGenBuffers(1, &m_id);
}

void VBO::bufferData(std::vector<Vertice>& vertices)
{
	m_vertexCount = vertices.size();
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertice) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
}

void VBO::allocateDynamicBuffer(GLuint size)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
}

void VBO::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}
void VBO::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}