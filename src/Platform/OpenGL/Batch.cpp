#include "Batch.h"

Batch::Batch(bool isPolygon)
	:m_vao()
	,m_vbo()
	,m_ibo()
	,m_vertBuffer()
	,m_indexBuffer()
	,m_indexCount(0)
	,m_isPolygon(isPolygon)
{
	m_vao.init();
	m_vbo.init();
	m_ibo.init();
	m_vao.bind();
	m_ibo.allocateDynamicBuffer(60000*20);
	m_vbo.allocateDynamicBuffer(sizeof(Vertice)*4*60000);
	m_vao.link(m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_vao.link(m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	m_vao.link(m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_vbo.unbind();
	m_vao.unbind();
}

Batch::~Batch()
{
	delete m_vertBuffer;
	delete m_indexBuffer;
}

void Batch::begin()
{
	m_vbo.bind();
	m_vertBuffer = (Vertice*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	m_ibo.bind();
	m_indexBuffer = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void Batch::submit(std::vector<Vertice> &vertices)
{
	if (m_indexCount + vertices.size()+1 >= (sizeof(Vertice) * 4 * 60000) - 1) flush();
	
	for (int i = 0; i < vertices.size(); i++)
	{
		m_vertBuffer->position = vertices[i].position;
		m_vertBuffer->color = vertices[i].color;
		m_vertBuffer->texCoord = vertices[i].texCoord;
		m_vertBuffer++;
		*m_indexBuffer = (GLuint)(i + m_indexCount);
		m_indexBuffer++;
	}
	*m_indexBuffer = (GLuint)MAGIX_NUMBER;
	m_indexBuffer++;
	m_indexCount += vertices.size();
}
void Batch::submit(std::vector<Vertice>& vertices, std::vector<GLuint> &indices)
{
	if (m_indexCount + indices.size() + 1 >= (sizeof(Vertice) * 4 * 60000) - 1) flush();
	for (int i = 0; i < vertices.size(); i++)
	{
		m_vertBuffer->position = vertices[i].position;
		m_vertBuffer->color = vertices[i].color;
		m_vertBuffer->texCoord = vertices[i].texCoord;
		m_vertBuffer++;
	}
	for (int i = 0; i < vertices.size(); i++)
	{
		*m_indexBuffer = (GLuint)(i+m_indexCount);
		m_indexBuffer++;
	}
	*m_indexBuffer = (GLuint)MAGIX_NUMBER;
	m_indexBuffer++;
	m_indexCount += indices.size();
}

void Batch::end()
{
	m_vbo.bind();
	glUnmapBuffer(GL_ARRAY_BUFFER);
	m_ibo.bind();
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	m_ibo.unbind();
}

void Batch::flush()
{
	if (m_indexCount == 0) return;

	GLuint drawType;
	if (!m_isPolygon) drawType = GL_LINE_STRIP;
	else drawType = GL_TRIANGLES;
	m_vao.bind();
	m_ibo.bind();
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(MAGIX_NUMBER);
	glDrawElements(drawType, m_indexCount, GL_UNSIGNED_INT, NULL);
	glDisable(GL_PRIMITIVE_RESTART);
	m_ibo.unbind();
	m_vao.unbind();
	m_indexCount = 0;
}
