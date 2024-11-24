#include "glad.h"
#include <GLFW/glfw3.h>
#include "CImg.h"
#include <iostream>
#include "OpenGLTexture.h"

using namespace cimg_library;

struct Point
{
    double x=0,y=0;
};

class ImageView
{
    public:
        Point center;
        double scale;
};

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec2 texOffset;
uniform float texScale;

void main()
{
    gl_Position = vec4(aPos*texScale + texOffset, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Texture Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Load image using CImg
    // CImg<unsigned char> image("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
    // int format = (image.spectrum() == 4) ? GL_RGBA : GL_RGB;
    //auto texture = BlueMarble::OpenGLTexture("/home/joar/git-repos/BlueMarbleMaps/geodata/blue_marble_256.jpg");
    //auto texture = BlueMarble::OpenGLTexture("/home/joar/git-repos/BlueMarbleMaps/geodata/elevation/LARGE_elevation.jpg");
    auto texture = BlueMarble::OpenGLTexture("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_LR_LC_SR_W/NE1_LR_LC_SR_W.jpg");
    ImageView view;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build and compile shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    static float centerX = 0.0f;
    static float centerY = 0.0f;
    static float scale = 1.0f;
    static bool isDragging;
    GLuint texOffsetLocation = glGetUniformLocation(shaderProgram, "texOffset");
    GLuint texScaleLocation = glGetUniformLocation(shaderProgram, "texScale");

    static double mouseX = 0.0, mouseY = 0.0;

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
    {
        constexpr double WHEEL_DELTA = 10.0;
        double s = 1.0 + abs(yOffset)/WHEEL_DELTA;
        double zoomFactor = yOffset > 0 ? s : 1.0/s;
        


        // int width, height;
        // glfwGetWindowSize(window, &width, &height);
        // double xpos, ypos;
		// glfwGetCursorPos(window, &xpos, &ypos);

        // double xNorm = xpos/(double)width*2.0 - 1.0;
        // double yNorm = 1.0 - ypos/(double)height*2.0;
        // //double mapX = ((double)x - sCenter.x()) / m_scale + m_center.x();
        // xNorm = xNorm/scale + centerX;
        // yNorm = yNorm/scale + centerY;

        // auto delta = Point{(xNorm - centerX), 
        //                    (yNorm - centerY)};
    
        // centerX = xNorm - delta.x*(1.0/zoomFactor);
        // centerY = yNorm - delta.y*(1.0/zoomFactor);
        scale *= zoomFactor;

        std::cout << "Scale: " << scale << "\n";
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
    {
        switch (action)
        {
            case GLFW_PRESS:
            {
                isDragging = true;
                break;
            }
            case GLFW_RELEASE:
            {
                isDragging = false;
                break;
            }
        }
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
    {
        if (isDragging)
        {
            double dx = mouseX-xPos;
            double dy = mouseY-yPos;

            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Convert to opengl compatible (-1, 1)
            double dxNorm = dx/(double)width*2.0;
            double dyNorm = -dy/(double)height*2.0;
            
            centerX += (float)dxNorm;
            centerY += (float)dyNorm;

            std::cout << "dx: " << dxNorm << ", " << "dy: " << dyNorm << "\n";
        }
        mouseX = xPos;
        mouseY = yPos;
    });

    // Set up vertex data and buffers
    float vertices[] = {
        // positions   // texture coords
        -.5f,  .5f,  0.0f, 1.0f, // top-left
        -.5f, -.5f,  0.0f, 0.0f, // bottom-left
         .5f, -.5f,  1.0f, 0.0f, // bottom-right
         .5f,  .5f,  1.0f, 1.0f  // top-right
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind texture and shader
        glUseProgram(shaderProgram);


        // Calculate some offset
        // int width, height;
        // glfwGetWindowSize(window, &width, &height);
        // double xpos, ypos;
		// glfwGetCursorPos(window, &xpos, &ypos);

        // double xNorm = xpos/(double)width*2.0 - 1.0;
        // double yNorm = 1.0 - ypos/(double)height*2.0;
		// //std::cout << "x: " << xNorm << ", y:" << yNorm << "\n";

        // centerX = xNorm; //cos(glfwGetTime()*0.5) * 0.15f;; // Example: move texture to the right
        // centerY = yNorm; //sin(glfwGetTime()) * 0.5f;
        
        // Set the texture offset uniform
        glUniform2f(texOffsetLocation, -centerX, -centerY);
        glUniform1f(texScaleLocation, scale);

        // TODO: Bind texture here
        //glBindTexture(GL_TEXTURE_2D, texture);
        texture.Bind();

        // Draw the rectangle
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}