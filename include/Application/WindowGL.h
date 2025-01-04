#pragma once
#include <string>
#include "glad/glad.h"
#include <glfw3.h>
class WindowGL
{
public:
	WindowGL();
	~WindowGL();
	// Returns false if initialization failed.
	bool init(int width, int height, std::string windowTitle);
	bool windowShouldClose();
	void shutdownWindow();
	void resizeWindow(int width, int height);
	void getMousePosition(double *xPos, double *yPos) const;
	void swapBuffers();
	/*
	* Registers a key event callback by taking in a callback function
	* function needs the integer parameters; key, scancode, action and modifier bits
	*/
	void registerKeyEventCallback(void callback(WindowGL* window,int key, int scanCode, int action, int modifier));
	//Resize callback which can be used by anyone who's interested in the window
	void registerResizeEventCallback(void callback(WindowGL* window, int width, int height));
	/*
	* This callback is only for the renderer. It is used to give the renderer a callback to when the windows framebuffer has been resized
	* This can be used to draw to the framebuffer while resizing
	*/
	void registerResizeFrameBufferEventCallback(void callback(WindowGL* window, int width, int height));
	//Click event for anyone interested in the window
	void registerMouseButtonEventCallback(void callback(WindowGL* window, int button, int action, int mods));
	//Mouse move event for anyone interested in the window
	void registerMousePositionEventCallback(void callback(WindowGL* window, double xPos, double yPos));
	//Mouse scroll event for anyone interested in the window
	void registerMouseScrollEventCallback(void callback(WindowGL* window, double xOffs, double yOffs));
	//Mouse leave and enter event for anyone interested in the window
	void registerMouseEnteredCallback(void callback(WindowGL* window, int entered));
	//Event for window closing
	void registerCloseWindowEventCallback(void callback(WindowGL* window));

	void pollWindowEvents();
	void waitWindowEvents();
	//timeout expressed in number of seconds.
	void waitWindowEventsTimeout(double timeout);
	GLFWwindow* getGLFWWindowHandle();
private:
	int m_width;
	int m_height;
	std::string m_windowTitle;
	GLFWwindow* m_window;

	void (*externalKeyEventCallback)(WindowGL*,int,int,int,int);
	void (*externalResizeEventCallback)(WindowGL*,int,int);
	void (*externalResizeFrameBufferEventCallback)(WindowGL*,int,int);
	void (*externalMouseButtonEventCallback)(WindowGL*,int,int,int);
	void (*externalMousePositionEventCallback)(WindowGL*,double,double);
	void (*externalMouseScrollEventCallback)(WindowGL*,double,double);
	void (*externalMouseEnteredEventCallback)(WindowGL*,int);
	void (*externalCloseWindowEventCallback)(WindowGL*);

	static void internalKeyEventCallback(GLFWwindow* window, int key, int scanCode, int action, int modifier);
	static void internalResizeEventCallback(GLFWwindow* window, int width, int height);
	static void internalResizeFrameBufferEventCallback(GLFWwindow* window, int width, int height);
	static void internalMouseButtonEventCallback(GLFWwindow* window, int button, int action, int mods);
	static void internalMousePositionEventCallback(GLFWwindow* window, double xPos, double yPos);
	static void internalMouseScrollEventCallback(GLFWwindow* window, double xOffs, double yOffs);
	static void internalMouseEnteredCallback(GLFWwindow* window, int entered);
	static void internalCloseWindowEventCallback(GLFWwindow* window);
};