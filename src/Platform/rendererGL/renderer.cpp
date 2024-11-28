#include "glad/glad.h"
#include <glfw3.h>
#include "glm.hpp"
#include <iostream>
#include "WindowGL.h"

void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier)
{
	std::cout << "I am inside your walls" << std::endl;
}
void resizeEvent(WindowGL* window, int width, int height)
{
	std::cout << "resize finished" << std::endl;
	glViewport(0,0,width,height);
}
void mouseButtonEvent(WindowGL* window, int button, int action, int modifier)
{
	std::cout << "received button event :)" << std::endl;
}
void mousePositionEvent(WindowGL* window, double x, double y)
{
	std::cout << "received mouse pos event" << std::endl;
}
void mouseScrollEvent(WindowGL* window, double xOffs, double yOffs)
{
	std::cout << "received scroll event" << std::endl;
}
void mouseEntered(WindowGL* window, int entered)
{
	std::cout << "received mouse entered event" << std::endl;
}
int main()
{
	WindowGL window;
	window.init(500,500,"Hello World");
	window.registerKeyEventCallback(keyEvent);
	window.registerResizeEventCallback(resizeEvent);
	window.registerMouseButtonEventCallback(mouseButtonEvent);
	window.registerMousePositionEventCallback(mousePositionEvent);
	window.registerMouseScrollEventCallback(mouseScrollEvent);
	window.registerMouseEnteredCallback(mouseEntered);

	window.resize(1000,1000);
	while (!window.windowShutdown())
	{
		// Keep running
		window.swapBuffers();
		window.pollWindowEvents();
	}
}
