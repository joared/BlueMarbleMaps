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

	/*
	* Registers a key event callback by taking in a callback function
	* function needs the integer parameters; key, scancode, action and modifier bits
	*/
	void registerKeyEventCallback(void callback(WindowGL* window,int key, int scanCode, int action, int modifier));

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

	static void internalKeyEventCallback(GLFWwindow* window, int key, int scanCode, int action, int modifier);
};