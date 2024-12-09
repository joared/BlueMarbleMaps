#include "glad/glad.h"
#include <glfw3.h>
#include <vector>
#include "glm.hpp"
#include <iostream>
#include "Application/WindowGL.h"
#include "Keys.h"

#include "Vertice.h"
#include "VAO.h"
#include "VBO.h"
#include "Shader.h"

void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier)
{
	Key keyStroke(scanCode);
	std::cout << "Key is: " << keyStroke << " " << keyStroke.toString() << keyStroke.isNumberKey() << keyStroke.numberKeyToInt() << std::endl;
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
	std::cout << "I shalle be doing a glViewPort resize yes" << std::endl;
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
void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}
int main()
{
	WindowGL window;
	if (!window.init(200, 200, "Hello World"))
	{
		std::cout << "Could not initiate window..." << std::endl;
	}
	
	glDebugMessageCallback(MessageCallback,0);

	window.registerKeyEventCallback(keyEvent);
	window.registerResizeEventCallback(resizeEvent);
	window.registerResizeFrameBufferEventCallback(resizeFrameBuffer);
	window.registerMouseButtonEventCallback(mouseButtonEvent);
	window.registerMousePositionEventCallback(mousePositionEvent);
	window.registerMouseScrollEventCallback(mouseScrollEvent);
	window.registerMouseEnteredCallback(mouseEntered);
	window.registerCloseWindowEventCallback(windowClosed);

	std::vector<Vertice> vertices = {Vertice(),Vertice(),Vertice()};
	vertices[0].position = glm::vec3(-0.5f,-0.5f,0.0f);
	vertices[1].position = glm::vec3(0.5f, -0.5f, 0.0f);
	vertices[2].position = glm::vec3(0.0f, 0.5f, 0.0f);

	Shader shader;

	shader.linkProgram("shaders\\basic.vert","shaders\\basic.frag");

	VBO vbo;
	VAO vao;

	vbo.init(vertices);
	vao.init();
	vao.bind();
	vao.link(vbo,0,3,GL_FLOAT,sizeof(Vertice), (void*)0);


	glClearColor(0.0f,0.0f,0.5f,1.0f);
	
	while (!window.windowShouldClose())
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		// Keep running

		shader.useProgram();
		vao.bind();
		glDrawArrays(GL_TRIANGLES,0,3);
		vao.unbind();
		window.swapBuffers();
		window.pollWindowEvents();
	}
	glfwTerminate();
}
