#pragma once
#include <glad/glad.h>
#include <BlueMarbleMaps/EngineObject.h>

#include <VAO.h>
#include <VBO.h>
#include <IBO.h>
#include <Shader.h>
#include <Texture.h>

class Primitive
{
public:
	Primitive();
	Primitive(VAO vao, VBO vbo, IBO ibo, Shader shader, Texture texture);
	~Primitive();

	void virtual setVao(VAO vao);
	void virtual setVbo(VBO vbo);
	void virtual setIbo(IBO ibo);
	void virtual setShader(Shader shader);
	void virtual setTexture(Texture texture);

	void virtual draw(GLenum drawType);

protected:
	VAO m_vao;
	VBO m_vbo;
	IBO m_ibo;
	Shader m_shader;
	Texture m_texture;
};