#include "Shader.h"

#include <fstream>
#include <streambuf>
Shader::Shader()
	:m_id(0)
{

}
Shader::~Shader()
{
	glDeleteProgram(m_id);
}

bool Shader::linkProgram(const char* vertexPath, const char* fragPath)
{
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vertexShaderSrc = readFile(vertexPath);
	glShaderSource(vertShader,1, &vertexShaderSrc, NULL);
	glCompileShader(vertShader);

	const char* fragShaderSrc = readFile(fragPath);
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);

	m_id = glCreateProgram();
	glAttachShader(m_id, vertShader);
	glAttachShader(m_id, fragShader);
	glLinkProgram(m_id);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
}
void Shader::useProgram()
{
	glUseProgram(m_id);
}

const char* Shader::readFile(const char* path)
{
	std::ifstream f(path);
	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return str.c_str();
}