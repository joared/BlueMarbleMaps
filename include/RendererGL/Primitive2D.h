#pragma once
#include <glad/glad.h>
#include <Primitive.h>

class Primitive2D : public Primitive
{
public:
	Primitive2D();
	Primitive2D(PrimitiveGeometryInfoPtr& info);
	~Primitive2D();

	void setVao(VAO& vao) override;
	void setVbo(VBO& vbo) override;
	void setIbo(IBO& ibo) override;
	void setShader(ShaderPtr shader) override;
	ShaderPtr getShader() override;
	void setTexture(TexturePtr texture) override;
	void setHasFill(bool fill) override;
	bool hasFill() override;
	void draw() override;
private:
	PrimitiveGeometryInfoPtr m_geometryInfo;
};
typedef std::shared_ptr<Primitive2D> Primitive2DPtr;
