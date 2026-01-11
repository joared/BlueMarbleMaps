#include "glad/glad.h"
#include <glfw3.h>
#include <iostream>

#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/Tools/ToolSet.h"
#include "BlueMarbleMaps/Core/Tools/DefaultEventHandlers.h"
#include "BlueMarbleMaps/Core/Tools/OttoTool.h"
#include "BlueMarbleMaps/Core/BlueMarbleLayout.h"
#include "Application/WindowGL.h"

#include "map_configuration.h"
#include <Keys.h>


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

    // TODO: map calls this and can be called on any thread
    void onUpdateRequest()
    {
        updateView();
        glfwPostEmptyEvent();
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
    // Configure some background layers
    configureMap(view);
    mapControl->setView(view);
    view->drawable()->backgroundColor(Color(120,170,255,0));

    //auto tool = std::make_shared<OttoTool>();
    auto toolSet = std::make_shared<ToolSet>();
    toolSet->addSubTool(std::make_shared<EditFeatureTool>());
    toolSet->addSubTool(std::make_shared<PointerTracerTool>());    
    toolSet->addSubTool(std::make_shared<KeyActionTool>());
    toolSet->addSubTool(std::make_shared<DebugEventHandler>());
    toolSet->addSubTool(std::make_shared<CameraControllerTwoHalfD>());
    mapControl->setTool(toolSet);

    mapControl->updateView();

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
