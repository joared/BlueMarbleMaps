#pragma once
#include <glad/glad.h>
#include <string>
#include <memory>
#include "glm.hpp"
struct Shader
{
	GLuint m_id;

	Shader();
	~Shader();
	
	bool linkProgram(const char* vertexPath, const char* fragPath);
	void useProgram();
	std::string readFile(const char* path);
	
	void setMat4(const char* uniform, glm::mat4& mat);
	void setMat3(const char* uniform, glm::mat3& mat);
	void setMat2(const char* uniform, glm::mat2& mat);
	void setvec4(const char* uniform, glm::vec4& vec);
	void setVec3(const char* uniform, glm::vec3& vec);
	void setVec2(const char* uniform, glm::vec2& vec);
	void setFloat(const char* uniform, GLfloat& f);
	void setInt(const char* uniform, GLint& i);

private:
	bool getShaderCompileLog(GLuint shaderId, char* info);
	bool getProgramLog(GLuint programId, char* info);
};

typedef std::shared_ptr<Shader> ShaderPtr;