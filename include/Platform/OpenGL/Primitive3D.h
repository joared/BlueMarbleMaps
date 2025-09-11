#pragma once
#include <glad/glad.h>
#include <Primitive.h>

class Primitive3D : public Primitive
{
public:
	Primitive3D();
	~Primitive3D();

	void setVao(VAO& vao) override;
	void setVbo(VBO& vbo) override;
	void setIbo(IBO& ibo) override;
	void setShader(Shader& shader) override;
	void setTexture(Texture& texture) override;

	void draw(GLenum drawType) override;

private:
	PrimitiveGeometryInfo m_geometryInfo;

};