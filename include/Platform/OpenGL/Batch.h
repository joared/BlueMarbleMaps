#pragma once
#include <memory>
#include <glad/glad.h>
#include <VAO.h>
#include <VBO.h>
#include <IBO.h>

class Batch
{
#define MAGIX_NUMBER 0xFFFFFFFF
public:
	Batch();
	~Batch();
	void begin();
	void submit(std::vector<Vertice>& vertices);
	void submit(std::vector<Vertice>& vertices, std::vector<GLuint>& indices);
	void end();
	void flush();

private:
	VAO m_vao;
	IBO m_ibo;
	VBO m_vbo;
	Vertice* m_vertBuffer;
	GLuint* m_indexBuffer;
	GLsizei m_indexCount;
};
typedef std::shared_ptr<Batch> BatchPtr;