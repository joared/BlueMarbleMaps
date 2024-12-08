#pragma once
#include <glad/glad.h>
#include <string>
struct Shader
{
	GLuint m_id;

	Shader();
	~Shader();
	
	bool linkProgram(const char* vertexPath, const char* fragPath);
	void useProgram();

	const char* readFile(const char* path);
	
};