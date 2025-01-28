#pragma once
#include <glad/glad.h>
#include <Primitive.h>

class Primitive2D : public Primitive
{
public:
	Primitive2D();
	~Primitive2D();

	void setVao(VAO vao) override;
	void setVbo(VBO vbo) override;
	void setIbo(IBO ibo) override;
	void setShader(Shader shader) override;
	void setTexture(Texture texture) override;

	void draw(GLenum drawType) override;

};