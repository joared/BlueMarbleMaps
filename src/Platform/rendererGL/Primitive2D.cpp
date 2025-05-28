#include "Primitive2D.h"

Primitive2D::Primitive2D()
	:m_geometryInfo()
{
}
Primitive2D::Primitive2D(PrimitiveGeometryInfoPtr& info)
	:m_geometryInfo(info)
{
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

void Primitive2D::draw()
{
	if (m_geometryInfo->m_texture != nullptr && m_geometryInfo->m_texture->m_id != 0)
	{
		m_geometryInfo->m_texture->bind();
	}
	m_geometryInfo->m_vao.bind();
	m_geometryInfo->m_vbo.bind();
	m_geometryInfo->m_ibo.bind();
	
	GLenum drawMode;
	if (hasFill()) drawMode = GL_TRIANGLES;
	else                          drawMode = GL_LINE;
	glDrawElements(drawMode, m_geometryInfo->m_ibo.m_nrOfIndices, GL_UNSIGNED_INT, 0);
	m_geometryInfo->m_vao.unbind();
	m_geometryInfo->m_vbo.unbind();
	m_geometryInfo->m_ibo.unbind();
	if (m_geometryInfo->m_texture != nullptr && m_geometryInfo->m_texture->m_id != 0)
	{
		m_geometryInfo->m_texture->unbind();
	}
}
