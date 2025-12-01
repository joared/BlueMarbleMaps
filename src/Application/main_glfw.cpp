#include "glad/glad.h"
#include <glfw3.h>
#include <iostream>

#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/DefaultEventHandlers.h"
#include "BlueMarbleMaps/Core/BlueMarbleLayout.h"
#include "Application/WindowGL.h"

#include "map_configuration.h"
#include <Keys.h>

#ifdef WIN32
#define PATH_TO_FANNY_FILE "../../../bluemarble_index/backgroundmap.png"
#else 
#define PATH_TO_FANNY_FILE "../../../bluemarble_index/backgroundmap.png"
#endif


using namespace BlueMarble;


class GLFWMapControl : public WindowGL, public MapControl
{
public:
    GLFWMapControl()
        : m_mouseDown(false)
        , m_wireFrameMode(false)
    {
    }

    int64_t setTimer(int64_t interval) override final
    {
        // TODO, not tested
        return -1;
    }

    bool killTimer(int64_t id) override final
    {
        // TODO, not tested
        return false;
    }

    void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier) override
    {
        Key keyStroke(scanCode);
        std::cout << "Key is: " << keyStroke << " " << keyStroke.toString() << "\n"; 
        if (keyStroke == Key::F && action == GLFW_PRESS)
        {
            m_wireFrameMode = !m_wireFrameMode;
            if (m_wireFrameMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else			  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (action == GLFW_PRESS)
        {
            keyDown(scanCode, getModificationKeyMask(), getGinotonicTimeStampMs());
        }
        else
        {
            keyUp(scanCode, getModificationKeyMask(), getGinotonicTimeStampMs());
        }
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

    MouseButton getMouseButton() const override final
    {
        MouseButton buttons = MouseButtonNone;
        int leftButton = glfwGetMouseButton(getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_LEFT);
        int rightButton = glfwGetMouseButton(getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_RIGHT);
        int middleButton = glfwGetMouseButton(getGLFWWindowHandle(), GLFW_MOUSE_BUTTON_MIDDLE);
        
        if (leftButton == GLFW_PRESS)
        {
            buttons = buttons | MouseButtonLeft;
        }

        if (rightButton == GLFW_PRESS)
        {
            buttons = buttons | MouseButtonRight;
        }

        if (middleButton == GLFW_PRESS)
        {
            buttons = buttons | MouseButtonMiddle;
        }

        return buttons;
    }

    ModificationKey getModificationKeyMask() const override final
    {
        auto window = getGLFWWindowHandle();
        ModificationKey modKeys = ModificationKeyNone;
        if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            modKeys = modKeys | ModificationKey::ModificationKeyCtrl;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            modKeys = modKeys | ModificationKey::ModificationKeyShift;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            modKeys = modKeys | ModificationKey::ModificationKeyAlt;
        }

        return modKeys;
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
            mouseDown(bmmButton, mousePos.x, mousePos.y, getModificationKeyMask(), getGinotonicTimeStampMs());
            m_mouseDown = true;
            break;
        }
        case GLFW_RELEASE:
        {
            mouseUp(bmmButton, mousePos.x, mousePos.y, getModificationKeyMask(), getGinotonicTimeStampMs());
            m_mouseDown = false;
            break;
        }
        }
    }

    void mousePositionEvent(WindowGL* window, double x, double y) override
    {
        mouseMove(getMouseButton(), x, y, getModificationKeyMask(), getGinotonicTimeStampMs());
    }
    
    void mouseScrollEvent(WindowGL* window, double xOffs, double yOffs) override
    {
        ScreenPos mousePos; getMousePos(mousePos);
        mouseWheel(yOffs, mousePos.x, mousePos.y, getModificationKeyMask(), getGinotonicTimeStampMs());
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
        bool m_wireFrameMode;
};
typedef std::shared_ptr<GLFWMapControl> GLFWMapControlPtr;

class EventObserver : public EventHandler
{
    public:
        EventObserver(const std::string& name)
            : EventHandler()
            , m_name(name)
            , m_previousEventType(EventType::Invalid)
        {}

        bool onEventFilter(const Event& event, EventHandler* target) override final
        {
            if (m_previousEventType != EventType::Invalid 
                && m_previousEventType != event.getType())
            {
                std::cout << m_name << ": " << event.toString() << "\n";
            }

            m_previousEventType = event.getType();

            if (event.getType() == EventType::DoubleClick)
            {
                return true;
            }
            return false;
        }
    private:
        std::string m_name;
        EventType   m_previousEventType;
};

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
    view->scale(1.0);
    view->showDebugInfo() = false;

    // auto elevationDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/readme/CompleteGeodata.png");
    auto elevationDataSet = std::make_shared<ImageDataSet>(PATH_TO_FANNY_FILE);
    
    elevationDataSet->initialize(DataSetInitializationType::RightHereRightNow);
    auto elevationLayer = std::make_shared<StandardLayer>(false);
    elevationLayer->addDataSet(elevationDataSet);
    auto rasterVis = std::make_shared<RasterVisualizer>();
    rasterVis->alpha(DirectDoubleAttributeVariable(0.7));
    elevationLayer->visualizers().push_back(rasterVis);
    view->addLayer(elevationLayer);

    configureMap(view, false, true, false);

    // Test Polygon/Line/Symbol visualizers
    auto vectorDataSet = std::make_shared<MemoryDataSet>();
    vectorDataSet->initialize(DataSetInitializationType::RightHereRightNow);

    auto polypoints = std::vector<Point>({{14,56}, {14,57}, {15,57}});
    auto linepoints = std::vector<Point>({{15,57}, {15,58}, {16,58}});
    auto testfeature = vectorDataSet->createFeature(std::make_shared<PolygonGeometry>(polypoints));
    auto testfeatureLine = vectorDataSet->createFeature(std::make_shared<LineGeometry>(linepoints));
    vectorDataSet->addFeature(testfeature);
    vectorDataSet->addFeature(testfeatureLine);
    using ReadOnlyFeaturePtr = std::shared_ptr<const Feature>;
    ReadOnlyFeaturePtr readOnlyFeature = testfeature;
    auto geom = readOnlyFeature->geometry();
    geom->moveTo({14,58});

    auto vectorLayer = std::make_shared<StandardLayer>(true);
    vectorLayer->addDataSet(vectorDataSet);
    vectorLayer->selectable(true);

    auto polyVis = std::make_shared<PolygonVisualizer>();
    vectorLayer->visualizers().push_back(polyVis);
    view->addLayer(vectorLayer);


    mapControl->setView(view);

    auto tool = std::make_shared<PanEventHandler>();
    tool->addSubTool(std::make_shared<EditFeatureTool>());
    tool->addSubTool(std::make_shared<PointerTracerTool>());
    tool->addSubTool(std::make_shared<KeyActionTool>());
    tool->addSubTool(std::make_shared<DebugEventHandler>());

    // EventObserver eventObserver1("Observer1");
    // EventObserver eventObserver2("Observer2");
    // tool.installEventFilter(&eventObserver1);
    // eventObserver1.installEventFilter(&eventObserver2);
    mapControl->setTool(tool);

    //view->crs(Crs::wgs84MercatorWeb());
    auto startRect = view->crs()->bounds();
    BMM_DEBUG() << "Start bounds: " << startRect.toString() << "\n";
    view->center(startRect.center());
    //view->center(Point(7727736.044037, 10090758.622196));
    view->width(startRect.width());
    //view->mapConstraints().bounds() = startRect;

    view->drawable()->backgroundColor(Color::gray(0.0));
    
    view->update(true);

    while (!mapControl->windowShouldClose())
    {
        mapControl->showFPS();
        if (mapControl->updateRequired())
        {
            mapControl->updateViewInternal();
            mapControl->pollWindowEvents();
        }
        else
        {
            mapControl->waitWindowEvents();
        }
    }
    glfwTerminate();

    return 0;
}
