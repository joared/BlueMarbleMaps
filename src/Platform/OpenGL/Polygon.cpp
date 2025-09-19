#include "Polygon.h"

Polygon::Polygon()
	:m_polygonGeometryInfo()
{
}

Polygon::Polygon(PolygonGeometryInfoPtr info, std::vector<Vertice>& vertices, std::vector<GLuint>& indices)
	:m_polygonGeometryInfo(info)
{
	if (vertices.empty() || indices.empty()) return;

	m_polygonGeometryInfo->m_vbo.init(vertices);
	m_polygonGeometryInfo->m_ibo.init(indices);
	m_polygonGeometryInfo->m_vao.init();

	m_polygonGeometryInfo->m_vao.bind();
	m_polygonGeometryInfo->m_vbo.bind();
	m_polygonGeometryInfo->m_vao.link(m_polygonGeometryInfo->m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_polygonGeometryInfo->m_vao.link(m_polygonGeometryInfo->m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	m_polygonGeometryInfo->m_vao.link(m_polygonGeometryInfo->m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_polygonGeometryInfo->m_vbo.unbind();
}

void Polygon::setVao(VAO& vao)
{
	m_polygonGeometryInfo->m_vao = vao;
}

void Polygon::setVbo(VBO& vbo)
{
	m_polygonGeometryInfo->m_vbo = vbo;
}

void Polygon::setIbo(IBO& ibo)
{
	m_polygonGeometryInfo->m_ibo = ibo;
}

void Polygon::setShader(ShaderPtr shader)
{
	m_polygonGeometryInfo->m_shader = shader;
}

ShaderPtr Polygon::getShader()
{
	return m_polygonGeometryInfo->m_shader;
}

void Polygon::setTexture(TexturePtr texture)
{
	m_polygonGeometryInfo->m_texture = texture;
}

void Polygon::setHasFill(bool fill)
{
	m_polygonGeometryInfo->m_hasFill = fill;
}

bool Polygon::hasFill()
{
	return m_polygonGeometryInfo->m_hasFill;
}

void Polygon::drawIndex(GLuint indexCount)
{
	if (m_polygonGeometryInfo->m_texture != nullptr && m_polygonGeometryInfo->m_texture->m_id != 0)
	{
		m_polygonGeometryInfo->m_texture->bind();
	}
	m_polygonGeometryInfo->m_vao.bind();
	m_polygonGeometryInfo->m_vbo.bind();
	m_polygonGeometryInfo->m_ibo.bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	m_polygonGeometryInfo->m_vao.unbind();
	m_polygonGeometryInfo->m_vbo.unbind();
	m_polygonGeometryInfo->m_ibo.unbind();
	if (m_polygonGeometryInfo->m_texture != nullptr && m_polygonGeometryInfo->m_texture->m_id != 0)
	{
		m_polygonGeometryInfo->m_texture->unbind();
	}
}

