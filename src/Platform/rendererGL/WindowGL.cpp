#include "WindowGL.h"

#include <iostream>

//Public functions

WindowGL::WindowGL()
	:m_width(0),
	m_height(0),
	m_windowTitle("")
{
	externalKeyEventCallback = nullptr;
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

	//Sets the this as the owner of the glfw window
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));

	//bind internal callbacks
	glfwSetKeyCallback(m_window, internalKeyEventCallback);

	return true;
}
bool WindowGL::windowShutdown()
{
	return glfwWindowShouldClose(m_window);
}

void WindowGL::registerKeyEventCallback(void callback(WindowGL* window, int key, int scanCode, int action, int modifier))
{
	externalKeyEventCallback = callback;
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

//private Functions

void WindowGL::internalKeyEventCallback(GLFWwindow* window, int key, int scanCode, int action, int modifier)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	if (*owner->externalKeyEventCallback != NULL)
	{
		std::cout << "function has been registered, am now calling function for one large mongoloid O____O" << std::endl;
		owner->externalKeyEventCallback(owner, key, scanCode, action, modifier);
	}
	else
	{
		std::cout << "umm... you need a function" << std::endl;
	}
}