#include "Line.h"

Line::Line()
	:m_lineGeometryInfo()
{
}

Line::Line(LineGeometryInfoPtr info, std::vector<Vertice>& vertices)
	:m_lineGeometryInfo(info)
{
	if (vertices.empty()) return;

	m_lineGeometryInfo->m_vbo.init();
	m_lineGeometryInfo->m_vbo.bufferData(vertices);
	m_lineGeometryInfo->m_vao.init();

	m_lineGeometryInfo->m_vao.bind();
	m_lineGeometryInfo->m_vbo.bind();
	m_lineGeometryInfo->m_vao.link(m_lineGeometryInfo->m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_lineGeometryInfo->m_vao.link(m_lineGeometryInfo->m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	//m_lineGeometryInfo->m_vao.link(m_lineGeometryInfo->m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_lineGeometryInfo->m_vbo.unbind();
}

void Line::setVao(VAO& vao)
{
	m_lineGeometryInfo->m_vao = vao;
}

void Line::setVbo(VBO& vbo)
{
	m_lineGeometryInfo->m_vbo = vbo;
}

void Line::setShader(ShaderPtr shader)
{
	m_lineGeometryInfo->m_shader = shader;
}

ShaderPtr Line::getShader()
{
	return m_lineGeometryInfo->m_shader;
}

void Line::drawIndex(GLuint indexCount)
{
	drawLine(m_lineGeometryInfo->m_vbo.m_vertexCount, 1);
}

void Line::drawLine(GLuint vertCount, float thickness)
{
	if (m_lineGeometryInfo->m_vao.m_id == 0 || m_lineGeometryInfo->m_vbo.m_id == 0) return;

	m_lineGeometryInfo->m_vao.bind();
	m_lineGeometryInfo->m_vbo.bind();
	glLineWidth(thickness);
	glDrawArrays(GL_LINE_STRIP, 0, vertCount);
	glLineWidth(1);
	m_lineGeometryInfo->m_vao.unbind();
	m_lineGeometryInfo->m_vbo.unbind();
}
