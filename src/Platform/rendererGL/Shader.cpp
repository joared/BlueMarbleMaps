#include "Shader.h"

#include <fstream>
#include <streambuf>
#include <iostream>
#include <gtc/type_ptr.hpp>
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

    std::string vertexShaderStr = readFile(vertexPath);
	const char* vertexShaderSrc = vertexShaderStr.c_str();
	glShaderSource(vertShader,1, &vertexShaderSrc, NULL);
	glCompileShader(vertShader);

	char infoLog[512];
	bool success = getShaderCompileLog(vertShader, infoLog);

	if (!success)
	{
		std::cout << "Could not load " << vertexPath << ":\n" << infoLog << std::endl;
	}
	
	std::string fragShaderStr = readFile(fragPath);
	const char* fragShaderSrc = fragShaderStr.c_str();
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);

	success = getShaderCompileLog(fragShader, infoLog);

	if (!success)
	{
		std::cout << "Could not load " << fragPath << ":\n" << infoLog << std::endl;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vertShader);
	glAttachShader(m_id, fragShader);
	glLinkProgram(m_id);

	success = getProgramLog(m_id, infoLog);

	if (!success)
	{
		std::cout << "Could not link program:\n" << infoLog << std::endl;
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return true;
}
void Shader::useProgram()
{
	glUseProgram(m_id);
}

std::string Shader::readFile(const char* path)
{
	std::ifstream f(path);
	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return str.c_str();
}

bool Shader::getShaderCompileLog(GLuint shaderId, char* info)
{
	int success = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shaderId, 512, NULL, info);
	}
	return success;
}
bool Shader::getProgramLog(GLuint programId, char* info)
{
	int success = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(programId, 512, NULL, info);
	}
	return success;
}

void Shader::setMat4(const char* uniform, glm::mat4& mat)
{
	glUniformMatrix4fv(glGetUniformLocation(m_id,uniform),1,false,glm::value_ptr(mat));
}
void Shader::setMat3(const char* uniform, glm::mat3& mat)
{
	glUniformMatrix3fv(glGetUniformLocation(m_id, uniform), 1, false, glm::value_ptr(mat));
}
void Shader::setMat2(const char* uniform, glm::mat2& mat)
{
	glUniformMatrix2fv(glGetUniformLocation(m_id, uniform), 1, false, glm::value_ptr(mat));
}
void Shader::setvec4(const char* uniform, glm::vec4& vec)
{
	glUniform4fv(glGetUniformLocation(m_id, uniform), 1, glm::value_ptr(vec));
}
void Shader::setVec3(const char* uniform, glm::vec3& vec)
{
	glUniform3fv(glGetUniformLocation(m_id, uniform), 1, glm::value_ptr(vec));
}
void Shader::setVec2(const char* uniform, glm::vec2& vec)
{
	glUniform2fv(glGetUniformLocation(m_id, uniform), 1, glm::value_ptr(vec));
}
void Shader::setFloat(const char* uniform, GLfloat& f)
{
	glUniform1fv(glGetUniformLocation(m_id, uniform), 1, &f);
}
void Shader::setInt(const char* uniform, GLint& i)
{
	glUniform1iv(glGetUniformLocation(m_id, uniform), 1, &i);
}