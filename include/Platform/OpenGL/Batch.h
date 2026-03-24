#pragma once
#include <memory>
#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#else
#include "glad/glad.h"
#endif
#include <VAO.h>
#include <VBO.h>
#include <IBO.h>

class Batch
{
#define MAGIX_NUMBER 0xFFFFFFFF
public:
	Batch(bool isPolygon);
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
	GLuint m_indexCount;
	GLuint m_verticeCounter;
	bool m_isPolygon;
};
typedef std::shared_ptr<Batch> BatchPtr;