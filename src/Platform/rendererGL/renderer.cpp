#include "glad/glad.h"
#include <glfw3.h>
#include <vector>
#include "glm.hpp"
#include <iostream>
#include "Application/WindowGL.h"
#include "Keys.h"
#include "stb_image.h"

#include "Vertice.h"
#include "VAO.h"
#include "VBO.h"
#include "IBO.h"
#include "Shader.h"
#include "Texture.h"
#include "CameraOrthographic.h"
#include "CameraPerspective.h"


class Renderer : public WindowGL
{
public:
	static CameraOrthographic cam;

	Renderer::Renderer()
	{
		stbi_set_flip_vertically_on_load(true);
		if (!WindowGL::init(1000, 1000, "Hello World"))
		{
			std::cout << "Could not initiate window..." << "\n";
		}

		glDebugMessageCallback(MessageCallback,0);

		const unsigned char* version = glGetString(GL_VERSION);
		std::cout << "opengl version: " << version << "\n";

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		std::vector<Vertice> vertices = {Vertice(),Vertice(),Vertice(),Vertice()};
		vertices[0].position = glm::vec3(-50.0f,-50.0f,0.0f);
		vertices[0].color = glm::vec4(0.1f, 0.7f, 0.0f,1.0f);
		vertices[0].texCoord = glm::vec2(0.0f,0.0f);
		vertices[1].position = glm::vec3(-50.0f, 50.0f, 0.0f);
		vertices[1].color = glm::vec4(0.9f, 0.1f, 0.9f, 1.0f);
		vertices[1].texCoord = glm::vec2(0.0f, 1.0f);
		vertices[2].position = glm::vec3(50.0f, 50.0f, 0.0f);
		vertices[2].color = glm::vec4(0.5f, 0.2f, 0.9f, 1.0f);
		vertices[2].texCoord = glm::vec2(1.0f, 1.0f);
		vertices[3].position = glm::vec3(50.0f, -50.0f, 0.0f);
		vertices[3].color = glm::vec4(0.5f, 0.8f, 0.3f, 1.0f);
		vertices[3].texCoord = glm::vec2(1.0f, 0.0f);

		std::vector<GLuint> indices = { 0,1,2,
										2,3,0 };

		Shader shader;

		shader.linkProgram("shaders/basic.vert","shaders/basic.frag");

		int imgWidth, imgHeight, imgChannels;

		unsigned char* image = readImage("C:/Users/Ottop/Onedrive/Skrivbord/goat.jpg", &imgWidth, &imgHeight, &imgChannels);

		GLint texIndex = 0;
		Texture tex;
		tex.init(image, imgWidth, imgHeight, imgChannels, GL_UNSIGNED_BYTE, texIndex);

		shader.useProgram();
		shader.setInt("texture0", texIndex);

		VBO vbo;
		VAO vao;
		IBO ibo;

		vbo.init(vertices);
		ibo.init(indices);
		vao.init();

		vao.bind();
		vbo.bind();
		vao.link(vbo,0, vertices.size(), GL_FLOAT, sizeof(Vertice), (void*)0);
		vao.link(vbo,1, vertices.size(), GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, color));
		vao.link(vbo,2, vertices.size(), GL_FLOAT, sizeof(Vertice), (void*)offsetof(Vertice, texCoord));
		vbo.unbind();

		glClearColor(0.1f,0.3f,0.2f,1.0f);

		cam.calculateTranslations();

		while (!windowShouldClose())
		{
			glClear(GL_COLOR_BUFFER_BIT);
			// Keep running

			shader.useProgram();
			shader.setMat4("viewMatrix",cam.getViewMatrix());
			vao.bind();
			ibo.bind();
			tex.bind();
			glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
			vao.unbind();
			vbo.unbind();
			ibo.unbind();
			tex.unbind();
			swapBuffers();
			pollWindowEvents();
		}
		glfwTerminate();
	}
	
	void Renderer::keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier) override
	{
		Key keyStroke(scanCode);
		static bool wireFrameMode = false;
		std::cout << "Key is: " << keyStroke << " " << keyStroke.toString() << "\n";
		if (keyStroke == Key::ESCAPE)
		{
			window->shutdownWindow();
		}
		else if (keyStroke == Key::F && action == GLFW_PRESS)
		{
			wireFrameMode = !wireFrameMode;
			if (wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else			  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else if (keyStroke == Key::W)
		{
			cam.pan(0, 5, 0);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::D)
		{
			cam.pan(5, 0, 0);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::S)
		{
			cam.pan(0, -5, 0);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::A)
		{
			cam.pan(-5, 0, 0);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::Z)
		{
			cam.pan(0, 0, 5);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::X)
		{
			cam.pan(0, 0, -5);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::Q)
		{
			cam.roll(10.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::E)
		{
			cam.roll(-10.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::K)
		{
			cam.yaw(5.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::L)
		{
			cam.yaw(-5.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::U)
		{
			cam.pitch(5.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::J)
		{
			cam.pitch(-5.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::O)
		{
			cam.zoom(5.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::P)
		{
			cam.zoom(-5.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::LEFT_ARROW)
		{
			cam.orbit(-0.1f, 0.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::RIGHT_ARROW)
		{
			cam.orbit(0.1f, 0.0f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::DOWN_ARROW)
		{
			cam.orbit(0.0f, -0.1f);
			cam.calculateTranslations();
		}
		else if (keyStroke == Key::UP_ARROW)
		{
			cam.orbit(0.0f, 0.1f);
			cam.calculateTranslations();
		}
	}
	void resizeEvent(WindowGL* window, int width, int height) override
	{
		std::cout << "resize finished" << "\n";
	}
	void resizeFrameBuffer(WindowGL* window, int width, int height) override
	{
		std::cout << "I shalle be doing a glViewPort resize yes" << "\n";
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT);
		window->swapBuffers();
		window->pollWindowEvents();
	}
	void mouseButtonEvent(WindowGL* window, int button, int action, int modifier) override
	{
		std::cout << "received button event :)" << "\n";
	}
	void mousePositionEvent(WindowGL* window, double x, double y) override
	{
		std::cout << "received mouse pos event" << "\n";
	}
	void mouseScrollEvent(WindowGL* window, double xOffs, double yOffs) override
	{
		std::cout << "received scroll event" << "\n";
	}
	void mouseEntered(WindowGL* window, int entered) override
	{
		std::cout << "received mouse entered event" << "\n";
	}
	void windowClosed(WindowGL* window) override
	{
		std::cout << "he's dead..." << "\n";
	}

	unsigned char* readImage(std::string path, int* width, int* height, int* nrOfChannels)
	{
		stbi_set_flip_vertically_on_load(true);

		unsigned char* imgBytes = stbi_load(path.c_str(), width, height, nrOfChannels, STBI_rgb_alpha);

		if (imgBytes == NULL)
		{
			std::cout << "couldn't load texture: " << stbi_failure_reason() << "\n";
			return nullptr;
		}
		return imgBytes;
	}

	static void GLAPIENTRY
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

	
};
CameraOrthographic Renderer::cam = CameraOrthographic(OrthographicCameraInformation());
//CameraPerspective Renderer::cam = CameraPerspective(PerspectiveCamerInformation());



int main()
{
	Renderer renderer;
}

