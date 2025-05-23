#include "VAO.h"

VAO::VAO()
	:m_id(0)
{

}
VAO::~VAO()
{

}
void VAO::init()
{
	glGenVertexArrays(1,&m_id);
}
void VAO::bind()
{
	glBindVertexArray(m_id);
}
void VAO::link(VBO& vbo, GLuint layout, GLuint nrOfComponents, GLenum type, GLsizeiptr stride, GLvoid* offset)
{
	glEnableVertexAttribArray(layout);
	glVertexAttribPointer(layout,nrOfComponents,type,GL_FALSE,stride,offset);
	glEnableVertexAttribArray(0);
}
void VAO::linkInt(VBO& vbo, GLuint layout, GLuint nrOfComponents, GLenum type, GLsizeiptr stride, GLvoid* offset)
{
	glEnableVertexAttribArray(layout);
	glVertexAttribIPointer(layout, nrOfComponents, type, stride, offset);
	glEnableVertexAttribArray(0);
}
void VAO::unbind()
{
	glBindVertexArray(0);
}