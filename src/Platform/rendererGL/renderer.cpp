#include "glad/glad.h"
#include <glfw3.h>
#include "glm.hpp"
#include <iostream>
#include "WindowGL.h"
#include "Keys.h"

void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier)
{
	Key keyStroke = static_cast<Key::KeyStroke>(scanCode);
	std::cout << "Key is: " << keyStroke << " " << keyStroke.toString() << std::endl;
	if (key == GLFW_KEY_ESCAPE)
	{
		window->shutdownWindow();
	}
}
void resizeEvent(WindowGL* window, int width, int height)
{
	std::cout << "resize finished" << std::endl;
}
void resizeFrameBuffer(WindowGL* window, int width, int height)
{
	std::cout << "I shalle be doing a glViwPort resize yes" << std::endl;
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);
	window->swapBuffers();
	window->pollWindowEvents();
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
void windowClosed(WindowGL* window)
{
	std::cout << "he's dead..." << std::endl;
}
int main()
{
	WindowGL window;
	if (!window.init(200, 200, "Hello World"))
	{
		std::cout << "Could not initiate window..." << std::endl;
	}
	
	window.registerKeyEventCallback(keyEvent);
	window.registerResizeEventCallback(resizeEvent);
	window.registerResizeFrameBufferEventCallback(resizeFrameBuffer);
	window.registerMouseButtonEventCallback(mouseButtonEvent);
	window.registerMousePositionEventCallback(mousePositionEvent);
	window.registerMouseScrollEventCallback(mouseScrollEvent);
	window.registerMouseEnteredCallback(mouseEntered);
	window.registerCloseWindowEventCallback(windowClosed);

	glClearColor(0.0f,0.0f,0.5f,1.0f);
	
	while (!window.windowShouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT);
		// Keep running
		window.swapBuffers();
		window.pollWindowEvents();
	}
	glfwTerminate();
}
