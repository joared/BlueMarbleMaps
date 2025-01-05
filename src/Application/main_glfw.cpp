#include "glad/glad.h"
#include <glfw3.h>
#include "CImg.h"
#include <iostream>

#include "Map.h"
#include "DataSet.h"
#include "Core.h"
#include "Feature.h"
#include "MapControl.h"
#include "DefaultEventHandlers.h"
#include "Application/WindowGL.h"

#include "map_configuration.h"
#include <Keys.h>

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

class GLFWMapControl :public WindowGL, public MapControl
{
public:
    GLFWMapControl()
        : m_mouseDown(false)
    {
    }
    void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier) override
    {
        Key keyStroke(scanCode);

    }

    void resizeEvent(WindowGL* window, int width, int height) override
    {
        GLFWMapControl::resize(width, height, getGinotonicTimeStampMs());
    }

    void resizeFrameBuffer(WindowGL* window, int width, int height) override
    {
        
    }

    void getMousePos(ScreenPos& pos) const override final
    {
        double x, y;
        getMousePosition(&x, &y);
        pos.x = (int)x;
        pos.y = (int)y;
    }

    void mouseButtonEvent(WindowGL* window, int button, int action, int modifier) override
    {
        ScreenPos mousePos; getMousePos(mousePos);
        switch (action)
        {
        case GLFW_PRESS:
        {
            mouseDown(MouseButtonLeft, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
            m_mouseDown = true;
            break;
        }
        case GLFW_RELEASE:
        {
            mouseUp(MouseButtonLeft, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
            m_mouseDown = false;
            break;
        }
        }
    }
    void mousePositionEvent(WindowGL* window, double x, double y) override
    {
        mouseMove(m_mouseDown ? MouseButtonLeft : MouseButtonNone, x, y, ModificationKeyNone, getGinotonicTimeStampMs());
    }
    void mouseScrollEvent(WindowGL* window, double xOffs, double yOffs) override
    {
        ScreenPos mousePos; getMousePos(mousePos);
        mouseWheel(yOffs, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
    }
    void mouseEntered(WindowGL* window, int entered) override
    {
        
    }
    void windowClosed(WindowGL* window) override
    {
        std::cout << "Window will close\n";
    }

    void* getWindow() override final
    {
        return (void*)getGLFWWindowHandle();
    }
    private:
        bool m_mouseDown;
};
typedef std::shared_ptr<GLFWMapControl> GLFWMapControlPtr;




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

int main() {
    //glDebugMessageCallback(MessageCallback, 0);
    //MapControlStuff
    auto mapControl = std::make_shared<GLFWMapControl>();
    if (!mapControl->init(1000, 1000, "Hello World"))
    {
        std::cout << "Could not initiate window..." << "\n";
    }
    glDebugMessageCallback(MessageCallback, 0);
    const unsigned char* version = glGetString(GL_VERSION);
    std::cout << "opengl version: " << version << "\n";

    auto view = std::make_shared<Map>();
    view->center(Point(0,0));
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

    std::vector<Point> points({ {16, 56}, {17, 57}, {15, 58} });
    auto poly = std::make_shared<PolygonGeometry>(points);
    vectorDataSet->addFeature(vectorDataSet->createFeature(poly));

    view->addLayer(&vectorLayer);

    PanEventHandler eventHandler(view, mapControl);
    mapControl->addSubscriber(&eventHandler);
    mapControl->setView(view);

    glDisable(GL_CULL_FACE);
    mapControl->updateViewInternal();
    while (!mapControl->windowShouldClose())
    {
        if (mapControl->updateRequired())
        {
            mapControl->updateViewInternal();
            mapControl->pollWindowEvents();
        }
        else
        {
            mapControl->waitWindowEvents();
        }
        //mapControl->swapBuffers();
    }
    glfwTerminate();

    return 0;
}
