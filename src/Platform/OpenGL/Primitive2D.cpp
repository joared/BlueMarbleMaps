#include "Platform/OpenGL/Primitive2D.h"

Primitive2D::Primitive2D()
	:m_geometryInfo()
{
}
Primitive2D::Primitive2D(PrimitiveGeometryInfoPtr& info, std::vector<Vertice> vertices)
	: m_geometryInfo(info)
{
	if (vertices.empty()) return;
	
	m_geometryInfo->m_vbo.init(vertices);
	m_geometryInfo->m_vao.init();

	m_geometryInfo->m_vao.bind();
	m_geometryInfo->m_vbo.bind();
	m_geometryInfo->m_vao.link(m_geometryInfo->m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_geometryInfo->m_vao.link(m_geometryInfo->m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	m_geometryInfo->m_vao.link(m_geometryInfo->m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_geometryInfo->m_vbo.unbind();
}
Primitive2D::Primitive2D(PrimitiveGeometryInfoPtr& info, std::vector<Vertice> vertices, std::vector<GLuint> indices)
	:m_geometryInfo(info)
{
	if (vertices.empty() || indices.empty()) return;

	m_geometryInfo->m_vbo.init(vertices);
	m_geometryInfo->m_ibo.init(indices);
	m_geometryInfo->m_vao.init();

	m_geometryInfo->m_vao.bind();
	m_geometryInfo->m_vbo.bind();
	m_geometryInfo->m_vao.link(m_geometryInfo->m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_geometryInfo->m_vao.link(m_geometryInfo->m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	m_geometryInfo->m_vao.link(m_geometryInfo->m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_geometryInfo->m_vbo.unbind();
}

Primitive2D::~Primitive2D()
{
}

void Primitive2D::setVao(VAO& vao)
{
	m_geometryInfo->m_vao = vao;
}

void Primitive2D::setVbo(VBO& vbo)
{
	m_geometryInfo->m_vbo = vbo;
}

void Primitive2D::setIbo(IBO& ibo)
{
	m_geometryInfo->m_ibo = ibo;
}

void Primitive2D::setShader(ShaderPtr shader)
{
	m_geometryInfo->m_shader = shader;
}

ShaderPtr Primitive2D::getShader()
{
	return m_geometryInfo->m_shader;
}

void Primitive2D::setTexture(TexturePtr texture)
{
	m_geometryInfo->m_texture = texture;
}

void Primitive2D::setHasFill(bool fill)
{
	m_geometryInfo->m_hasFill = fill;
}

bool Primitive2D::hasFill()
{
	return m_geometryInfo->m_hasFill;
}

void Primitive2D::drawIndex(GLuint indexCount)
{
	if (m_geometryInfo->m_texture != nullptr && m_geometryInfo->m_texture->m_id != 0)
	{
		m_geometryInfo->m_texture->bind();
	}
	m_geometryInfo->m_vao.bind();
	m_geometryInfo->m_vbo.bind();
	m_geometryInfo->m_ibo.bind();
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	m_geometryInfo->m_vao.unbind();
	m_geometryInfo->m_vbo.unbind();
	m_geometryInfo->m_ibo.unbind();
	if (m_geometryInfo->m_texture != nullptr && m_geometryInfo->m_texture->m_id != 0)
	{
		m_geometryInfo->m_texture->unbind();
	}
}
void Primitive2D::drawLine(GLuint vertCount, float thickness)
{
	m_geometryInfo->m_vao.bind();
	m_geometryInfo->m_vbo.bind();
	glLineWidth(thickness);
	glDrawArrays(GL_LINE_STRIP, 0, vertCount);
	glLineWidth(1);
	m_geometryInfo->m_vao.unbind();
	m_geometryInfo->m_vbo.unbind();
}
