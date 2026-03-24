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
	// if (vertices.size() != 4)
	// {
	// 	std::cout << "WARNING: ";	
	// 	std::cout << "VBO Size: " << vertices.size() << "\n";
	// }
	
	// glBindBuffer(GL_ARRAY_BUFFER, m_id);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(Vertice) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	m_vertexCount = vertices.size();

	GLint size = vertices.size() * sizeof(Vertice);
	// std::cout << "Uploading buffer size: " << size << "\n";

	glBufferData(GL_ARRAY_BUFFER, size, vertices.data(), GL_DYNAMIC_DRAW);

	// Immediately verify what GPU thinks:
	GLint actualSize = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &actualSize);
	// std::cout << "Actual GPU buffer size: " << actualSize << "\n";
	if (size != actualSize)
	{
		std::cout << "WARNING: CPU/GPU buffer size diff: " << size << " != " << actualSize << "\n";
	}
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