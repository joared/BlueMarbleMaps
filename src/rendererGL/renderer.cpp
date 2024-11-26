#include "glad/glad.h"
#include <glfw3.h>
#include "glm.hpp"

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(500,500,"Fuck you", nullptr,nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	while (true)
	{

	}
}