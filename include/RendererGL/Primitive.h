#pragma once
#include <glad/glad.h>
#include <BlueMarbleMaps/Core/EngineObject.h>

#include <VAO.h>
#include <VBO.h>
#include <IBO.h>
#include <Shader.h>
#include <Texture.h>

struct PrimitiveGeometryInfo
{
	VAO m_vao;
	VBO m_vbo;
	IBO m_ibo;
	Shader m_shader;
	Texture m_texture;
	bool m_hasFill = true;
};

class Primitive
{
public:
	Primitive() = default;
	virtual ~Primitive() = default;

	void virtual setVao(VAO& vao) = 0;
	void virtual setVbo(VBO& vbo) = 0;
	void virtual setIbo(IBO& ibo) = 0;
	void virtual setShader(Shader& shader) = 0;
	void virtual setTexture(Texture& texture) = 0;
	void virtual setHasFill(bool fill) = 0;
	bool virtual hasFill() = 0;
	void virtual draw(GLenum drawType) = 0;
	
};