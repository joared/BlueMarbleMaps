#include "Batch.h"
#include <iostream>


#ifdef __EMSCRIPTEN__
#define BATCH_SIZE 600000
#else
#define BATCH_SIZE 600000
#endif

Batch::Batch(bool isPolygon)
	:m_vao()
	,m_vbo()
	,m_ibo()
	,m_vertBuffer()
	,m_indexBuffer()
	,m_indexCount(0)
	,m_verticeCounter(0)
	,m_isPolygon(isPolygon)
{
	#ifdef __EMSCRIPTEN__
	m_cpuVertices.reserve(BATCH_SIZE);
	m_cpuIndices.reserve(BATCH_SIZE);
	m_offsets.reserve(BATCH_SIZE);
	m_counts.reserve(BATCH_SIZE);
	#endif

	m_vao.init();
	m_vbo.init();
	m_ibo.init();
	m_vao.bind();
	m_ibo.allocateDynamicBuffer(sizeof(GLuint)*BATCH_SIZE);
	m_vbo.allocateDynamicBuffer(sizeof(Vertice)*BATCH_SIZE);
	m_vao.link(m_vbo, 0, 3, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, position));
	m_vao.link(m_vbo, 1, 4, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
	m_vao.link(m_vbo, 2, 2, GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
	m_vbo.unbind();
	m_vao.unbind();
}

Batch::~Batch()
{
	// Results in "invalid pointer" errors
	// delete m_vertBuffer;
	// delete m_indexBuffer;
}

void Batch::begin()
{
	#ifdef __EMSCRIPTEN__
	m_cpuVertices.clear();
    m_cpuIndices.clear();
	#else
	m_vbo.bind();
	m_vertBuffer = (Vertice*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	m_ibo.bind();
	m_indexBuffer = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	#endif
}
//Issue, on extremely large polygons how should we handle magix_number.
//the "flush end begin should also be moved to a continuous check every time we draw something"
void Batch::submit(std::vector<Vertice> &vertices)
{
	#ifdef __EMSCRIPTEN__

	if (vertices.size() == 0) return;

	if (m_cpuVertices.size()+vertices.size() >= (BATCH_SIZE))
	{
		std::cout << "flushing in submit" << "\n";
		flush();
		end();
		begin();
	}

	size_t indexOffset = m_cpuIndices.size();
	size_t vertexCounter = m_cpuVertices.size();
	for (int i = 0; i < vertices.size(); i++)
	{
		m_cpuVertices.push_back(vertices[i]);
		m_cpuIndices.push_back(i + vertexCounter);
	}

	// 4. Store draw info
    m_counts.push_back(static_cast<GLsizei>(vertices.size()));

    // IMPORTANT: offset is in BYTES, not index count
    m_offsets.push_back((void*)(indexOffset * sizeof(GLuint)));

	#else
	if (m_indexCount + vertices.size()+1 >= (BATCH_SIZE) - 1)
	{
		std::cout << "flushing in submit" << "\n";
		end();
		flush();
		begin();
	}
	if (m_indexCount != 0)
	{
		*m_indexBuffer = (GLuint)MAGIX_NUMBER;
		m_indexBuffer++;
		m_indexCount++;
	}
	for (int i = 0; i < vertices.size(); i++)
	{
		m_vertBuffer->position = vertices[i].position;
		m_vertBuffer->color = vertices[i].color;
		m_vertBuffer->texCoord = vertices[i].texCoord;
		m_vertBuffer++;
		*m_indexBuffer = (GLuint)(i + m_verticeCounter);
		m_indexBuffer++;
	}
	m_indexCount += vertices.size();
	m_verticeCounter += vertices.size();
	
	#endif
}

void Batch::submit(std::vector<Vertice>& vertices, std::vector<GLuint> &indices)
{
	if (vertices.size() == 0 || indices.size() == 0) return;

	#ifdef __EMSCRIPTEN__

	if (m_cpuVertices.size()+vertices.size() >= (BATCH_SIZE))
	{
		std::cout << "flushing in submit" << "\n";
		flush();
		end();
		begin();
	}

	size_t indexOffset = m_cpuIndices.size();
	size_t vertexCounter = m_cpuVertices.size();
	for (int i = 0; i < vertices.size(); i++)
	{
		m_cpuVertices.push_back(vertices[i]);
	}

	for (int i = 0; i < indices.size(); i++)
	{
		m_cpuIndices.push_back(indices[i] + vertexCounter);
	}

	// 4. Store draw info
    m_counts.push_back(static_cast<GLsizei>(indices.size()));

    // IMPORTANT: offset is in BYTES, not index count
    m_offsets.push_back((void*)(indexOffset * sizeof(GLuint)));

	#else

	if (m_indexCount + indices.size() + 1 >= (BATCH_SIZE) - 1) 
	{
		std::cout << "flushing in submit" << "\n";
		flush();
		end();
		begin();
	}
	if (m_indexCount != 0)
	{
		*m_indexBuffer = (GLuint)MAGIX_NUMBER;
		m_indexBuffer++;
		m_indexCount++;
	}
	for (int i = 0; i < vertices.size(); i++)
	{
		m_vertBuffer->position = vertices[i].position;
		m_vertBuffer->color = vertices[i].color;
		m_vertBuffer->texCoord = vertices[i].texCoord;
		m_vertBuffer++;
	}
	for (int i = 0; i < indices.size(); i++)
	{
		*m_indexBuffer = (GLuint)(indices[i] + m_verticeCounter);
		m_indexBuffer++;
	}
    m_indexCount += indices.size();
	m_verticeCounter += vertices.size();
	#endif
}

void Batch::end()
{
	
	m_vbo.bind();
	glUnmapBuffer(GL_ARRAY_BUFFER);
	m_ibo.bind();
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	m_ibo.unbind();
}

void Batch::flush()
{
	#ifdef __EMSCRIPTEN__
	m_vao.bind();
    m_vbo.bind();
    m_ibo.bind();

    glBufferSubData(GL_ARRAY_BUFFER, 0,
        m_cpuVertices.size() * sizeof(Vertice),
        m_cpuVertices.data());

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
        m_cpuIndices.size() * sizeof(GLuint),
        m_cpuIndices.data());

    GLenum drawType = m_isPolygon ? GL_TRIANGLES : GL_LINE_STRIP;

	size_t offset = 0;
	for (size_t i = 0; i < m_counts.size(); ++i) {
		glDrawElements(drawType, m_counts[i], GL_UNSIGNED_INT,
					   (void*)(offset * sizeof(GLuint)));
		offset += m_counts[i];
	}
	// Not supported in GLES
	// glMultiDrawElements(
	// 	drawType,
	// 	m_counts.data(),
	// 	GL_UNSIGNED_INT,
	// 	m_offsets.data(),
	// 	m_counts.size()
	// );

	m_cpuIndices.clear();
	m_cpuIndices.clear();
	m_counts.clear();
	m_offsets.clear();

	#else
	if (m_indexCount == 0) return;

	GLuint drawType;
	if (!m_isPolygon) drawType = GL_LINE_STRIP;
	else drawType = GL_TRIANGLES;
	m_vao.bind();
	m_ibo.bind();
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(MAGIX_NUMBER);
	glDrawElements(drawType, m_indexCount, GL_UNSIGNED_INT, NULL);
	glDisable(GL_PRIMITIVE_RESTART);
	m_ibo.unbind();
	m_vao.unbind();
	m_indexCount = 0;
	m_verticeCounter = 0;
	#endif
}
