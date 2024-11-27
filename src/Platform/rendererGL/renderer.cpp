#include "glad/glad.h"
#include <glfw3.h>
#include "glm.hpp"
#include <iostream>
#include "WindowGL.h"

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(500,500,"Fuck you", nullptr,nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	while (!glfwWindowShouldClose(window))
	{
		// Keep running
		double fuck = glm::acos(0.1);
		std::cout << fuck << "\n";
		glfwPollEvents();
	}
}