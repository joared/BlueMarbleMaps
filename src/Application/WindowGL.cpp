#include "Application/WindowGL.h"

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
	glfwTerminate();
	glfwDestroyWindow(m_window);
}

bool WindowGL::init(int width, int height, std::string windowTitle)
{
	
	if (!glfwInit())
	{
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//Sets the this pointer as the owner of the glfw window
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));
	glEnable(GL_DEBUG_OUTPUT);

	//bind internal callbacks
	glfwSetKeyCallback(m_window, internalKeyEventCallback);
	glfwSetWindowSizeCallback(m_window, internalResizeEventCallback);
	glfwSetFramebufferSizeCallback(m_window, internalResizeFrameBufferEventCallback);
	glfwSetMouseButtonCallback(m_window, internalMouseButtonEventCallback);
	glfwSetCursorPosCallback(m_window, internalMousePositionEventCallback);
	glfwSetScrollCallback(m_window, internalMouseScrollEventCallback);
	glfwSetCursorEnterCallback(m_window, internalMouseEnteredCallback);
	glfwSetWindowCloseCallback(m_window, internalCloseWindowEventCallback);

	return true;
}
bool WindowGL::windowShouldClose()
{
	return glfwWindowShouldClose(m_window);
}
void WindowGL::shutdownWindow()
{
	glfwSetWindowShouldClose(m_window,GLFW_TRUE);
}
void WindowGL::swapBuffers()
{
	glfwSwapBuffers(m_window);
}
/*
void WindowGL::registerKeyEventCallback(void callback(WindowGL* window, int key, int scanCode, int action, int modifier))
{
	externalKeyEventCallback = callback;
}

void WindowGL::registerResizeEventCallback(void callback(WindowGL* window, int width, int height))
{
	externalResizeEventCallback = callback;
}

void WindowGL::registerResizeFrameBufferEventCallback(void callback(WindowGL* window, int width, int height))
{
	externalResizeFrameBufferEventCallback = callback;
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
void WindowGL::registerCloseWindowEventCallback(void callback(WindowGL* window))
{
	externalCloseWindowEventCallback = callback;
}
*/
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

void WindowGL::resizeWindow(int width, int height)
{
	glfwSetWindowSize(m_window, width, height);
}

void WindowGL::getMousePosition(double* xPos, double* yPos) const
{
	glfwGetCursorPos(m_window,xPos,yPos);
}

//private Functions

void WindowGL::internalKeyEventCallback(GLFWwindow* window, int key, int scanCode, int action, int modifier)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "function has been registered, am now calling function for one large mongoloid O____O" << std::endl;
	owner->keyEvent(owner, key, scanCode, action, modifier);
}

void WindowGL::internalResizeEventCallback(GLFWwindow* window, int width, int height)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "Resizing mai balls to precisely x: " << width << " y: " << height << std::endl;
	owner->resizeEvent(owner, width, height);
}

void WindowGL::internalResizeFrameBufferEventCallback(GLFWwindow* window, int width, int height)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "gosh darn it resize your view port to x: " << width << " y: " << height << std::endl;
	owner->resizeFrameBuffer(owner, width, height);
}

void WindowGL::internalMouseButtonEventCallback(GLFWwindow* window, int button, int action, int modifier)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "mouse button baby: " << button << std::endl;
	owner->mouseButtonEvent(owner, button, action, modifier);
}

void WindowGL::internalMousePositionEventCallback(GLFWwindow* window, double xPos, double yPos)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	//std::cout << "mouse position x: " << xPos << " y: " << yPos << " on my balls" << std::endl;
	owner->mousePositionEvent(owner, xPos, yPos);
}
void WindowGL::internalMouseScrollEventCallback(GLFWwindow* window, double xOffs, double yOffs)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "mouse scroll x: " << xOffs << " y: " << yOffs << std::endl;
	owner->mouseScrollEvent(owner, xOffs, yOffs);
}

void WindowGL::internalMouseEnteredCallback(GLFWwindow* window, int entered)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "mouse in " << entered << std::endl;
	owner->mouseEntered(owner, entered);
}

void WindowGL::internalCloseWindowEventCallback(GLFWwindow* window)
{
	WindowGL* owner = reinterpret_cast<WindowGL*>(glfwGetWindowUserPointer(window));
	std::cout << "I am going to die" << std::endl;
	owner->windowClosed(owner);
}

GLFWwindow* WindowGL::getGLFWWindowHandle()
{
	return m_window;
}