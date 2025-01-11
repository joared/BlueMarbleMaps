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
        MouseButton bmmButton;

        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                bmmButton = MouseButtonLeft;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                bmmButton = MouseButtonRight;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                bmmButton = MouseButtonMiddle;
                break;
            default:
                std::cout << "Unknown mouse button: " << button << "\n";
                return;
        }

        switch (action)
        {
        case GLFW_PRESS:
        {
            mouseDown(bmmButton, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
            m_mouseDown = true;
            break;
        }
        case GLFW_RELEASE:
        {
            mouseUp(bmmButton, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
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


int main() 
{
    //MapControlStuff
    auto mapControl = std::make_shared<GLFWMapControl>();
    if (!mapControl->init(1000, 1000, "Hello World"))
    {
        std::cout << "Could not initiate window..." << "\n";
    }
    //glDebugMessageCallback(MessageCallback, 0);
    const unsigned char* version = glGetString(GL_VERSION);
    std::cout << "opengl version: " << version << "\n";

    auto view = std::make_shared<Map>();
    view->center(Point(0, 0));
    view->scale(0.1);
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
        std::cout << view->center().toString() << "\n";
        //mapControl->swapBuffers();
    }
    glfwTerminate();

    return 0;
}
