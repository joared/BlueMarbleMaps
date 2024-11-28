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
	bool windowShutdown();
	void resize(int width, int height);
	void getMousePosition(double *xPos, double *yPos);
	void swapBuffers();
	/*
	* Registers a key event callback by taking in a callback function
	* function needs the integer parameters; key, scancode, action and modifier bits
	*/
	void registerKeyEventCallback(void callback(WindowGL* window,int key, int scanCode, int action, int modifier));
	void registerResizeEventCallback(void callback(WindowGL* window, int width, int height));
	void registerMouseButtonEventCallback(void callback(WindowGL* window, int button, int action, int mods));
	void registerMousePositionEventCallback(void callback(WindowGL* window, double xPos, double yPos));
	void registerMouseScrollEventCallback(void callback(WindowGL* window, double xOffs, double yOffs));
	void registerMouseEnteredCallback(void callback(WindowGL* window, int entered));

	void pollWindowEvents();
	void waitWindowEvents();
	//timeout expressed in number of seconds.
	void waitWindowEventsTimeout(double timeout);

private:
	int m_width;
	int m_height;
	std::string m_windowTitle;
	GLFWwindow* m_window;

	void (*externalKeyEventCallback)(WindowGL*, int, int, int, int);
	void (*externalResizeEventCallback)(WindowGL*,int,int);
	void (*externalMouseButtonEventCallback)(WindowGL*,int,int,int);
	void (*externalMousePositionEventCallback)(WindowGL*, double, double);
	void (*externalMouseScrollEventCallback)(WindowGL*, double, double);
	void(*externalMouseEnteredEventCallback)(WindowGL*,int);

	static void internalKeyEventCallback(GLFWwindow* window, int key, int scanCode, int action, int modifier);
	static void internalResizeEventCallback(GLFWwindow* window, int width, int height);
	static void internalMouseButtonEventCallback(GLFWwindow* window, int button, int action, int mods);
	static void internalMousePositionEventCallback(GLFWwindow* window, double xPos, double yPos);
	static void internalMouseScrollEventCallback(GLFWwindow* window, double xOffs, double yOffs);
	static void internalMouseEnteredCallback(GLFWwindow* window, int entered);
};