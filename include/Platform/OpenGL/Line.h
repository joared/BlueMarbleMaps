#pragma once
#include "Primitive.h"

struct LineGeometryInfo
{
	LineGeometryInfo()
		: m_vao()
		, m_vbo()
		, m_shader()
	{

	}
	LineGeometryInfo(VAO &vao, VBO &vbo, IBO &ibo, ShaderPtr shader)
		: m_vao(vao)
		, m_vbo(vbo)
		, m_shader(shader)
	{

	}
	VAO m_vao;
	VBO m_vbo;
	ShaderPtr m_shader;
};
typedef std::shared_ptr<LineGeometryInfo> LineGeometryInfoPtr;

class Line : public Primitive
{
public:
	Line();
	Line(LineGeometryInfoPtr info, std::vector<Vertice> &vertices);

	void setVao(VAO& vao) override;
	void setVbo(VBO& vbo) override;
	//doesn't do dick
	void setIbo(IBO& ibo) {};
	void setShader(ShaderPtr shader) override;
	ShaderPtr getShader() override;
	//doesn't do dick
	void setTexture(TexturePtr texture) {};
	//doesn't do dick
	void setHasFill(bool fill) {};
	bool hasFill() override { return false; }
	//Only calls drawline
	void drawIndex(GLuint indexCount) override;
	void drawLine(GLuint vertCount, float thickness) override;

private:
	LineGeometryInfoPtr m_lineGeometryInfo;
};
typedef std::shared_ptr<Line> LinePtr;