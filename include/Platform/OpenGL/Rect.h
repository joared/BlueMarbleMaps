#pragma once
#include "Primitive.h"

struct RectGeometryInfo
{
	RectGeometryInfo()
		: m_vao()
		, m_vbo()
		, m_ibo()
		, m_shader()
		, m_texture()
		, m_hasFill()
	{

	}
	RectGeometryInfo(VAO& vao, VBO& vbo, IBO& ibo, ShaderPtr shader)
		: m_vao(vao)
		, m_vbo(vbo)
		, m_ibo(ibo)
		, m_shader(shader)
		, m_texture()
		, m_hasFill()
	{

	}
	RectGeometryInfo(VAO& vao, VBO& vbo, IBO& ibo, ShaderPtr shader, TexturePtr texture, bool hasFill = true)
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
typedef std::shared_ptr<RectGeometryInfo> RectGeometryInfoPtr;

class Rect : public Primitive
{
public:
	Rect();
	Rect(RectGeometryInfoPtr info, std::vector<Vertice>& vertices, std::vector<GLuint>& indices);

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
	RectGeometryInfoPtr m_rectGeometryInfo;
};
typedef std::shared_ptr<Rect> RectPtr;