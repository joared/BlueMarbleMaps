#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "CImg.h"
#include <iostream>
#include "OpenGLTexture.h"

#include "Map.h"
#include "DataSet.h"
#include "Core.h"
#include "Feature.h"
#include "MapControl.h"
#include "DefaultEventHandlers.h"

#include "map_configuration.h"

using namespace cimg_library;
using namespace BlueMarble;


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

class GLFWMapControl : public MapControl
{
    public:
        GLFWMapControl(GLFWwindow* window)
            : m_window(window) 
        {
            glfwSetWindowUserPointer(window, this);

            glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
            {
                auto* instance = static_cast<GLFWMapControl*>(glfwGetWindowUserPointer(window));
                instance->resize(width, height, getGinotonicTimeStampMs());
            });

            glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                auto* instance = static_cast<GLFWMapControl*>(glfwGetWindowUserPointer(window));
                ScreenPos mousePos; instance->getMousePos(mousePos);
                instance->mouseWheel(yOffset, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
            });

            glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
            {
                auto* instance = static_cast<GLFWMapControl*>(glfwGetWindowUserPointer(window));
                int x,y = 0;
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        instance->mouseDown(MouseButtonLeft, x, y, ModificationKeyNone, getGinotonicTimeStampMs());
                        break;
                    }
                    case GLFW_RELEASE:
                    {
                        instance->mouseUp(MouseButtonLeft, x, y, ModificationKeyNone, getGinotonicTimeStampMs());
                        break;
                    }
                }
            });

            glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
            {
                auto* instance = static_cast<GLFWMapControl*>(glfwGetWindowUserPointer(window));
                instance->mouseMove(MouseButtonLeft, xPos, yPos, ModificationKeyNone, getGinotonicTimeStampMs());
            });
        }



        void getMousePos(ScreenPos& pos) const override final
        {
            double x,y;
            glfwGetCursorPos(m_window, &x, &y);
            pos.x = (int)x;
            pos.y = (int)y;
        }

        void* getWindow() override final
        {
            return (void*)m_window;
        }
    private:
        GLFWwindow* m_window;
};
typedef std::shared_ptr<GLFWMapControl> GLFWMapControlPtr;


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

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    auto mapControl = std::make_shared<GLFWMapControl>(window);
    auto view = std::make_shared<Map>();

    auto elevationDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/elevation/LARGE_elevation.jpg");
    elevationDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
    auto elevationLayer = BlueMarble::Layer(false);
    elevationLayer.addUpdateHandler(elevationDataSet.get());
    auto rasterVis = std::make_shared<RasterVisualizer>();
    rasterVis->alpha(DirectDoubleAttributeVariable(0.5));
    elevationLayer.visualizers().push_back(rasterVis);
    view->addLayer(&elevationLayer);

    // Test Polygon/Line/Symbol visualizers
    auto vectorDataSet = std::make_shared<BlueMarble::MemoryDataSet>();
    vectorDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
    auto vectorLayer = BlueMarble::Layer(true);
    vectorLayer.addUpdateHandler(vectorDataSet.get());
    
    std::vector<Point> points({{16, 56}, {17, 57}, {15, 58}});
    auto poly = std::make_shared<PolygonGeometry>(points);
    vectorDataSet->addFeature(vectorDataSet->createFeature(poly));

    view->addLayer(&vectorLayer);

    // Stress test by adding more layers and visualization using this
    configureMap(view); 

    // Load image using CImg
    // CImg<unsigned char> image("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
    // int format = (image.spectrum() == 4) ? GL_RGBA : GL_RGB;
    //auto texture = BlueMarble::OpenGLTexture("/home/joar/git-repos/BlueMarbleMaps/geodata/blue_marble_256.jpg");
    //auto texture = BlueMarble::OpenGLTexture("/home/joar/git-repos/BlueMarbleMaps/geodata/elevation/LARGE_elevation.jpg");
    auto texture = BlueMarble::OpenGLTexture("/home/joar/git-repos/BlueMarbleMaps/geodata/elevation/LARGE_elevation.jpg");

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

    GLuint texOffsetLocation = glGetUniformLocation(shaderProgram, "texOffset");
    GLuint texScaleLocation = glGetUniformLocation(shaderProgram, "texScale");

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

// #include <iostream>
// #include "CImg.h"

// #include "Map.h"
// #include "DataSet.h"
// #include "Core.h"
// #include "Feature.h"
// #include "MapControl.h"
// #include "glad/glad.h"
// #include <glfw3.h>

// #include "DefaultEventHandlers.h"

// #include "map_configuration.h"

// using namespace cimg_library;
// using namespace BlueMarble;

// class GLFWMapControl : public MapControl
// {
//     public:
//         GLFWMapControl() {}
//     private:
//         GLFWwindow* m_window;
// };

// class CImgMapControl : public MapControl
// {
//     public:
//         CImgMapControl(CImgDisplay& display)
//             : m_disp(display)
//         {}

//         bool getResize(int& width, int& height) override final
//         {
//             if (m_disp.is_resized() || m_disp.is_keyF11())
//             {
//                 std::cout << "Resize: " << m_disp.window_width() << ", " << m_disp.window_height() << "\n";
//                 width = m_disp.window_width();
//                 height = m_disp.window_height();
                
//                 return true;
//             }

//             return false;
//         }

//         int getWheelDelta() override final
//         {
//             int delta = m_disp.wheel();
//             m_disp.set_wheel();
//             return delta;
//         }

//         // TODO: remove this and use keyCode instead
//         bool captureKeyEvents() override final
//         {
//             handleKey(m_disp.is_keyARROWDOWN(), KeyButton::ArrowDown);
//             handleKey(m_disp.is_keyARROWUP(), KeyButton::ArrowUp);
//             handleKey(m_disp.is_keyARROWLEFT(), KeyButton::ArrowLeft);
//             handleKey(m_disp.is_keyARROWRIGHT(), KeyButton::ArrowRight);
//             handleKey(m_disp.is_keyENTER(), KeyButton::Enter);
//             handleKey(m_disp.is_keySPACE(), KeyButton::Space);
//             handleKey(m_disp.is_keyBACKSPACE(), KeyButton::BackSpace);
//             handleKey(m_disp.is_key1(), KeyButton::One);
//             handleKey(m_disp.is_key2(), KeyButton::Two);
//             handleKey(m_disp.is_key3(), KeyButton::Three);
//             handleKey(m_disp.is_key4(), KeyButton::Four);
//             handleKey(m_disp.is_key5(), KeyButton::Five);
//             handleKey(m_disp.is_key6(), KeyButton::Six);
//             handleKey(m_disp.is_key7(), KeyButton::Seven);
//             handleKey(m_disp.is_key8(), KeyButton::Eight);
//             handleKey(m_disp.is_key9(), KeyButton::Nine);
//             handleKey(m_disp.is_keyPADADD(), KeyButton::Add);
//             handleKey(m_disp.is_keyPADSUB(), KeyButton::Subtract);

//             return true;
//         }
//         void getMousePos(ScreenPos &pos) const override final
//         {
//             pos.x = m_disp.mouse_x();
//             pos.y = m_disp.mouse_y();
//         }

//         ModificationKey getModificationKeyMask() const override final
//         {
//             ModificationKey keyMask = ModificationKeyNone;
//             if (m_disp.is_keySHIFTLEFT() || m_disp.is_keySHIFTRIGHT())
//             {
//                 keyMask = keyMask | ModificationKeyShift;
//             }

//             if (m_disp.is_keyCTRLLEFT() || m_disp.is_keyCTRLRIGHT())
//             {
//                 keyMask = keyMask | ModificationKeyCtrl;
//             }

//             if (m_disp.is_keyALT())
//             {
//                 keyMask = keyMask | ModificationKeyAlt;
//             }

//             return keyMask;

//         }
//         MouseButton getMouseButton() override final
//         {
//             unsigned int button = m_disp.button();
//             MouseButton mouseButton = MouseButtonNone;
//             if (button & 0x1)
//             {
//                 // Left mouse button
//                 mouseButton = MouseButtonLeft;
//             }
//             else if (button & 0x2)
//             {
//                 // Right mouse button
//                 mouseButton = MouseButtonRight;
//             }
//             else if (button & 0x4)
//             {
//                 // Middle mouse button
//                 mouseButton = MouseButtonMiddle;
//             }

//             return mouseButton;
//         }

//         void* getWindow() override final
//         {
//             return (void*)&m_disp;
//         }
//     private:
//         CImgDisplay& m_disp;
// };
// typedef std::shared_ptr<CImgMapControl> CImgMapControlPtr;


// int main()
// {
//     // Set up window/display
//     BlueMarble::MapPtr map = std::make_shared<Map>();
//     int normalization = 0;
//     CImgDisplay display(cimg_library::CImg<unsigned char>(), "BlueMarbleMaps Demo", normalization, true, true); //*static_cast<CImgDisplay*>(map->drawable()->getDisplay());
//     display.resize(500, 500, true);

//     // Test RasterVisualizer
//     //auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
//     //auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/nasa/eo_base_2020_clean_geo.tif");
//     // auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/blue_marble_256.jpg");
//     // backgroundDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
//     // auto backgroundLayer = BlueMarble::Layer(false);
//     // auto rasterVis1 = std::make_shared<RasterVisualizer>();
//     // rasterVis1->alpha(DirectDoubleAttributeVariable(1.0));
//     // backgroundLayer.visualizers().push_back(rasterVis1);
//     // backgroundLayer.addUpdateHandler(backgroundDataSet.get());
//     // map->addLayer(&backgroundLayer);

//     auto elevationDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/elevation/LARGE_elevation.jpg");
//     elevationDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
//     auto elevationLayer = BlueMarble::Layer(false);
//     elevationLayer.addUpdateHandler(elevationDataSet.get());
//     auto rasterVis = std::make_shared<RasterVisualizer>();
//     rasterVis->alpha(DirectDoubleAttributeVariable(0.5));
//     elevationLayer.visualizers().push_back(rasterVis);
//     map->addLayer(&elevationLayer);

//     // Test Polygon/Line/Symbol visualizers
//     auto vectorDataSet = std::make_shared<BlueMarble::MemoryDataSet>();
//     vectorDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
//     auto vectorLayer = BlueMarble::Layer(true);
//     vectorLayer.addUpdateHandler(vectorDataSet.get());
    
//     std::vector<Point> points({{16, 56}, {17, 57}, {15, 58}});
//     auto poly = std::make_shared<PolygonGeometry>(points);
//     vectorDataSet->addFeature(vectorDataSet->createFeature(poly));

//     map->addLayer(&vectorLayer);

//     // Stress test by adding more layers and visualization using this
//     configureMap(map); 

//     // Setup MapControl and event handlers
//     CImgMapControlPtr mapControl = std::make_shared<CImgMapControl>(display);
//     auto panHandler = BlueMarble::PanEventHandler(*map, mapControl);
//     mapControl->addSubscriber(&panHandler);
//     mapControl->setView(map);
    
//     // Main loop
//     //map->startInitialAnimation();
//     map->update();
//     while (!display.is_closed() && !display.is_keyESC()) 
//     {
//         mapControl->captureEvents();
//         if (display.is_resized() || display.is_keyF11())
//         {
//             display.resize(false);
//         }


//         if (mapControl->updateRequired())
//         {
//             mapControl->updateViewInternal();
//         }
//         else
//         {
//             display.wait();
//         }
//     }
    
//     return 0;
// }