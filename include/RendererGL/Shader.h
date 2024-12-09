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
	std::string readFile(const char* path);
	
private:
	bool getShaderCompileLog(GLuint shaderId, char* info);
	bool getProgramLog(GLuint programId, char* info);
};