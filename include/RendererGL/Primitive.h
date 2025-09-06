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
	PrimitiveGeometryInfo()
		: m_vao()
		, m_vbo()
		, m_ibo()
		, m_shader()
		, m_texture()
	{
		
	}
	VAO m_vao;
	VBO m_vbo;
	IBO m_ibo;
	ShaderPtr m_shader;
	TexturePtr m_texture;
	bool m_hasFill = true;
};
typedef std::shared_ptr<PrimitiveGeometryInfo> PrimitiveGeometryInfoPtr;

class Primitive
{
public:
	Primitive() = default;
	virtual ~Primitive() = default;

	virtual void setVao(VAO& vao) = 0;
	virtual void setVbo(VBO& vbo) = 0;
	virtual void setIbo(IBO& ibo) = 0;
	virtual void setShader(ShaderPtr shader) = 0;
	virtual ShaderPtr getShader() = 0;
	virtual void setTexture(TexturePtr texture) = 0;
	virtual void setHasFill(bool fill) = 0;
	virtual bool hasFill() = 0;
	virtual void drawIndex(GLuint indexCount) = 0;
	virtual void drawLine(GLuint vertCount, float thickness) = 0;
	
};
typedef std::shared_ptr<Primitive> PrimitivePtr;
