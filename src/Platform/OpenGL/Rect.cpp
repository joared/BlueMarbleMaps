#include "Rect.h"

Rect::Rect()
	:m_rectGeometryInfo()
{
}

Rect::Rect(RectGeometryInfoPtr info, std::vector<Vertice>& vertices, std::vector<GLuint>& indices)
	:m_rectGeometryInfo(info)
{
	if (vertices.empty() || indices.empty()) return;

	m_rectGeometryInfo->m_vbo.init();
	m_rectGeometryInfo->m_vbo.bufferData(vertices);
	m_rectGeometryInfo->m_ibo.init();
	m_rectGeometryInfo->m_ibo.bufferData(indices);
	m_rectGeometryInfo->m_vao.init();

	m_rectGeometryInfo->m_vao.bind();
	m_rectGeometryInfo->m_vbo.bind();
	m_rectGeometryInfo->m_vao.link(m_rectGeometryInfo->m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_rectGeometryInfo->m_vao.link(m_rectGeometryInfo->m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	m_rectGeometryInfo->m_vao.link(m_rectGeometryInfo->m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_rectGeometryInfo->m_vbo.unbind();
}

void Rect::setVao(VAO& vao)
{
	m_rectGeometryInfo->m_vao = vao;
}

void Rect::setVbo(VBO& vbo)
{
	m_rectGeometryInfo->m_vbo = vbo;
}

void Rect::setIbo(IBO& ibo)
{
	m_rectGeometryInfo->m_ibo = ibo;
}

void Rect::setShader(ShaderPtr shader)
{
	m_rectGeometryInfo->m_shader = shader;
}

ShaderPtr Rect::getShader()
{
	return m_rectGeometryInfo->m_shader;
}

void Rect::setTexture(TexturePtr texture)
{
	m_rectGeometryInfo->m_texture = texture;
}

void Rect::setHasFill(bool fill)
{
	m_rectGeometryInfo->m_hasFill = fill;
}

bool Rect::hasFill()
{
	return m_rectGeometryInfo->m_hasFill;
}

void Rect::drawIndex(GLuint indexCount)
{
	if (m_rectGeometryInfo->m_vao.m_id == 0 || m_rectGeometryInfo->m_vbo.m_id == 0 || m_rectGeometryInfo->m_ibo.m_id == 0) return;
	{
		m_rectGeometryInfo->m_texture->bind();
	}
	m_rectGeometryInfo->m_vao.bind();
	m_rectGeometryInfo->m_vbo.bind();
	m_rectGeometryInfo->m_ibo.bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	m_rectGeometryInfo->m_vao.unbind();
	m_rectGeometryInfo->m_vbo.unbind();
	m_rectGeometryInfo->m_ibo.unbind();
	if (m_rectGeometryInfo->m_texture != nullptr && m_rectGeometryInfo->m_texture->m_id != 0)
	{
		m_rectGeometryInfo->m_texture->unbind();
	}
}

