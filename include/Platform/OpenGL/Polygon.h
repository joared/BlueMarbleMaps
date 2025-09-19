#pragma once
#include "Primitive.h"

struct PolygonGeometryInfo
{
	PolygonGeometryInfo()
		: m_vao()
		, m_vbo()
		, m_ibo()
		, m_shader()
		, m_texture()
	{

	}
	PolygonGeometryInfo(VAO& vao, VBO& vbo, IBO& ibo, ShaderPtr shader, bool hasFill = true)
		: m_vao(vao)
		, m_vbo(vbo)
		, m_ibo(ibo)
		, m_shader(shader)
		, m_hasFill(hasFill)
		, m_texture()
	{

	}
	PolygonGeometryInfo(VAO& vao, VBO& vbo, IBO& ibo, ShaderPtr shader, TexturePtr texture, bool hasFill = true)
		: m_vao(vao)
		, m_vbo(vbo)
		, m_ibo(ibo)
		, m_shader(shader)
		, m_texture(texture)
		, m_hasFill(hasFill)
	{

	}
	VAO m_vao;
	VBO m_vbo;
	IBO m_ibo;
	ShaderPtr m_shader;
	TexturePtr m_texture;
	bool m_hasFill = true;
};
typedef std::shared_ptr<PolygonGeometryInfo> PolygonGeometryInfoPtr;

class Polygon : public Primitive
{
public:
	Polygon();
	Polygon(PolygonGeometryInfoPtr info, std::vector<Vertice>& vertices, std::vector<GLuint>& indices);

	void setVao(VAO& vao) override;
	void setVbo(VBO& vbo) override;
	void setIbo(IBO& ibo) override;
	void setShader(ShaderPtr shader) override;
	ShaderPtr getShader() override;
	void setTexture(TexturePtr texture) override;
	void setHasFill(bool fill) override;
	bool hasFill() override;
	void drawIndex(GLuint indexCount) override;
	//doesn't do dick
	void drawLine(GLuint vertCount, float thickness) {};
private:
	PolygonGeometryInfoPtr m_polygonGeometryInfo;
};
typedef std::shared_ptr<Polygon> PolygonPtr;