#include "glad/glad.h"
#include <glfw3.h>
#include "glm.hpp"
#include <iostream>
#include "WindowGL.h"

void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier)
{
	std::cout << "I am inside your walls" << std::endl;
}

int main()
{
	WindowGL window;
	window.init(500,500,"Hello World");
	window.registerKeyEventCallback(keyEvent);
	while (!window.windowShutdown())
	{
		// Keep running
		double fuck = glm::acos(0.1);
		
		window.pollWindowEvents();
	}
}
