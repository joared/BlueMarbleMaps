#pragma once
#include <glad/glad.h>
#include "Platform/OpenGL/Primitive.h"

class Primitive2D : public Primitive
{
public:
	Primitive2D();
	Primitive2D(PrimitiveGeometryInfoPtr& info, std::vector<Vertice> vertices);
	Primitive2D(PrimitiveGeometryInfoPtr& info, std::vector<Vertice> vertices, std::vector<GLuint> indices);
	~Primitive2D();

	void setVao(VAO& vao) override;
	void setVbo(VBO& vbo) override;
	void setIbo(IBO& ibo) override;
	void setShader(ShaderPtr shader) override;
	ShaderPtr getShader() override;
	void setTexture(TexturePtr texture) override;
	void setHasFill(bool fill) override;
	bool hasFill() override;
	void drawIndex(GLuint indexCount) override;
	void drawLine(GLuint vertCount, float thickness) override;
private:
	PrimitiveGeometryInfoPtr m_geometryInfo;
};
typedef std::shared_ptr<Primitive2D> Primitive2DPtr;
