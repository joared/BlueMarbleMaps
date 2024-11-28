#include "WindowGL.h"

#include <iostream>

//Public functions

WindowGL::WindowGL()
	:m_width(0),
	m_height(0),
	m_windowTitle("")
{

}
WindowGL::~WindowGL()
{
	delete(m_window);
}

bool WindowGL::init(int width, int height, std::string windowTitle)
{
	glfwInit();
	m_window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	//Sets the this pointer as the owner of the glfw window
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));

	//bind internal callbacks
	glfwSetKeyCallback(m_window, internalKeyEventCallback);
	glfwSetWindowSizeCallback(m_window, internalResizeEventCallback);
	glfwSetMouseButtonCallback(m_window, internalMouseButtonEventCallback);
	glfwSetCursorPosCallback(m_window, internalMousePositionEventCallback);
	glfwSetScrollCallback(m_window, internalMouseScrollEventCallback);
	glfwSetCursorEnterCallback(m_window, internalMouseEnteredCallback);

	return true;
}
bool WindowGL::windowShutdown()
{
	return glfwWindowShouldClose(m_window);
}

void WindowGL::swapBuffers()
{
	glfwSwapBuffers(m_window);
}

void WindowGL::registerKeyEventCallback(void callback(WindowGL* window, int key, int scanCode, int action, int modifier))
{
	externalKeyEventCallback = callback;
}

void WindowGL::registerResizeEventCallback(void callback(WindowGL* window, int width, int height))
{
	externalResizeEventCallback = callback;
}

void WindowGL::registerMouseButtonEventCallback(void callback(WindowGL* window, int button, int action, int mods))
{
	externalMouseButtonEventCallback = callback;
}

void WindowGL::registerMousePositionEventCallback(void callback(WindowGL* window, double xPos, double yPos))
{
	externalMousePositionEventCallback = callback;
}
void WindowGL::registerMouseScrollEventCallback(void callback(WindowGL* window, double xOffs, double yOffs))
{
	externalMouseScrollEventCallback = callback;
}
void WindowGL::registerMouseEnteredCallback(void callback(WindowGL* window, int entered))
{
	externalMouseEnteredEventCallback = callback;
}

void WindowGL::pollWindowEvents()
{
	glfwPollEvents();
}
void WindowGL::waitWindowEvents()
{
	glfwWaitEvents();
}
void WindowGL::waitWindowEventsTimeout(double timeout)
{
	glfwWaitEventsTimeout(timeout);
}

void WindowGL::resize(int width, int height)
{
	glfwSetWindowSize(m_window, width, height);
}

void WindowGL::getMousePosition(double* xPos, double* yPos)
{
	glfwGetCursorPos(m_window,xPos,yPos);
}

//private Functions

void WindowGL::internalKeyEventCallback(GLFWwindow* window, int key, int scanCode, int action, int modifier)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalKeyEventCallback != NULL)
	{
		std::cout << "function has been registered, am now calling function for one large mongoloid O____O" << std::endl;
		owner->externalKeyEventCallback(owner, key, scanCode, action, modifier);
	}
}

void WindowGL::internalResizeEventCallback(GLFWwindow* window, int width, int height)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalResizeEventCallback != NULL)
	{
		std::cout << "Resizing mai balls to precisely y: " << height << " x: " << width << std::endl;
		owner->externalResizeEventCallback(owner, width, height);
	}
}

void WindowGL::internalMouseButtonEventCallback(GLFWwindow* window, int button, int action, int modifier)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalMouseButtonEventCallback != NULL)
	{
		std::cout << "mouse button baby: " << button << std::endl;
		owner->externalMouseButtonEventCallback(owner, button, action, modifier);
	}
}

void WindowGL::internalMousePositionEventCallback(GLFWwindow* window, double xPos, double yPos)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalMousePositionEventCallback != NULL)
	{
		std::cout << "mouse position x: " << xPos << " y: " << yPos << " on my balls" << std::endl;
		owner->externalMousePositionEventCallback(owner, xPos, yPos);
	}
}
void WindowGL::internalMouseScrollEventCallback(GLFWwindow* window, double xOffs, double yOffs)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalMouseScrollEventCallback != NULL)
	{
		std::cout << "mouse scroll x: " << xOffs << " y: " << yOffs << std::endl;
		owner->externalMouseScrollEventCallback(owner, xOffs, yOffs);
	}
}

void WindowGL::internalMouseEnteredCallback(GLFWwindow* window, int entered)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalMouseEnteredEventCallback != NULL)
	{
		std::cout << "mouse in " << entered << std::endl;
		owner->externalMouseEnteredEventCallback(owner, entered);
	}
}