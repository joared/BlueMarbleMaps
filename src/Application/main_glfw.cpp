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
    {
        static auto key_static = [this](WindowGL* window, int key, int scancode, int action, int mods) {
                // because we have a this pointer we are now able to call a non-static member method:
            keyEvent(window, key, scancode, action, mods);
        };
        static auto resize_static = [this](WindowGL * window, int width, int height) {
            resizeEvent(window, width, height);
        };
        static auto resizeFrame_static = [this](WindowGL* window, int width, int height) {
            resizeFrameBuffer(window, width, height);
        };
        static auto mouseButton_static = [this](WindowGL* window, int button, int action, int modifier) {
            mouseButtonEvent(window,button,action,modifier);
        };
        static auto mousePosition_static = [this](WindowGL* window, double x, double y) {
            mousePositionEvent(window, x, y);
        };
        static auto mouseScroll_static = [this](WindowGL* window, double xOffs, double yOffs) {
            mouseScrollEvent(window, xOffs, yOffs);
        };
        static auto mouseEntered_static = [this](WindowGL* window, int entered) {
            mouseEntered(window, entered);
        };
        static auto windowClose_static = [this](WindowGL* window) {
            windowClosed(window);
        };
        registerKeyEventCallback([](WindowGL* window, int key, int scancode, int action, int mods) {key_static(window, key, scancode, action, mods); });
        registerResizeEventCallback([](WindowGL* window, int width, int height) {resize_static(window, width, height); });
        registerResizeFrameBufferEventCallback([](WindowGL* window, int width, int height) {resizeFrame_static(window, width, height); });
        registerMouseButtonEventCallback([](WindowGL* window, int button, int action, int modifier) {mouseButton_static(window, button, action, modifier); });
        registerMousePositionEventCallback([](WindowGL* window, double x, double y) { mousePosition_static(window,x,y);});
        registerMouseScrollEventCallback([](WindowGL* window, double xOffs, double yOffs) { mouseScroll_static(window,xOffs,yOffs);});
        registerMouseEnteredCallback([](WindowGL* window, int entered) { mouseEntered_static(window, entered);});
        registerCloseWindowEventCallback([](WindowGL* window) { windowClose_static(window);});
    }
    void keyEvent(WindowGL* window, int key, int scanCode, int action, int modifier)
    {
        Key keyStroke(scanCode);

    }

    void resizeEvent(WindowGL* window, int width, int height)
    {
        GLFWMapControl::resize(width, height, getGinotonicTimeStampMs());
    }

    void resizeFrameBuffer(WindowGL* window, int width, int height)
    {
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);
        window->swapBuffers();
        window->pollWindowEvents();
    }

    void getMousePos(ScreenPos& pos) const override final
    {
        double x, y;
        getMousePosition(&x, &y);
        pos.x = (int)x;
        pos.y = (int)y;
    }

    void mouseButtonEvent(WindowGL* window, int button, int action, int modifier)
    {
        int x = 0, y = 0;
        switch (action)
        {
        case GLFW_PRESS:
        {
            mouseDown(MouseButtonLeft, x, y, ModificationKeyNone, getGinotonicTimeStampMs());
            break;
        }
        case GLFW_RELEASE:
        {
            mouseUp(MouseButtonLeft, x, y, ModificationKeyNone, getGinotonicTimeStampMs());
            break;
        }
        }
    }
    void mousePositionEvent(WindowGL* window, double x, double y)
    {
        mouseMove(MouseButtonLeft, x, y, ModificationKeyNone, getGinotonicTimeStampMs());
    }
    void mouseScrollEvent(WindowGL* window, double xOffs, double yOffs)
    {
        ScreenPos mousePos; getMousePos(mousePos);
        mouseWheel(yOffs, mousePos.x, mousePos.y, ModificationKeyNone, getGinotonicTimeStampMs());
    }
    void mouseEntered(WindowGL* window, int entered)
    {
        
    }
    void windowClosed(WindowGL* window)
    {
        std::cout << "Window will close\n";
    }

    void* getWindow() override final
    {
        return (void*)getGLFWWindowHandle();
    }
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
    const unsigned char* version = glGetString(GL_VERSION);
    std::cout << "opengl version: " << version << "\n";
    auto view = std::make_shared<Map>();

    auto elevationDataSet = std::make_shared<BlueMarble::ImageDataSet>("C:/Users/Ottop/Onedrive/skrivbord/goat.jpg");
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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    while (!mapControl->windowShouldClose())
    {
        
        mapControl->swapBuffers();
        mapControl->pollWindowEvents();
    }
    glfwTerminate();

    return 0;
}
/*
 #include <iostream>
 #include "CImg.h"

 #include "Map.h"
 #include "DataSet.h"
 #include "Core.h"
 #include "Feature.h"
 #include "MapControl.h"
 #include "glad/glad.h"
 #include <glfw3.h>

 #include "DefaultEventHandlers.h"

 #include "map_configuration.h"

 using namespace cimg_library;
 using namespace BlueMarble;

 class GLFWMapControl : public MapControl
 {
     public:
         GLFWMapControl() {}
     private:
         GLFWwindow* m_window;
 };

 class CImgMapControl : public MapControl
 {
     public:
         CImgMapControl(CImgDisplay& display)
             : m_disp(display)
         {}

         bool getResize(int& width, int& height) override final
         {
             if (m_disp.is_resized() || m_disp.is_keyF11())
             {
                 std::cout << "Resize: " << m_disp.window_width() << ", " << m_disp.window_height() << "\n";
                 width = m_disp.window_width();
                 height = m_disp.window_height();
                
                 return true;
             }

             return false;
         }

         int getWheelDelta() override final
         {
             int delta = m_disp.wheel();
             m_disp.set_wheel();
             return delta;
         }

         // TODO: remove this and use keyCode instead
         bool captureKeyEvents() override final
         {
             handleKey(m_disp.is_keyARROWDOWN(), KeyButton::ArrowDown);
             handleKey(m_disp.is_keyARROWUP(), KeyButton::ArrowUp);
             handleKey(m_disp.is_keyARROWLEFT(), KeyButton::ArrowLeft);
             handleKey(m_disp.is_keyARROWRIGHT(), KeyButton::ArrowRight);
             handleKey(m_disp.is_keyENTER(), KeyButton::Enter);
             handleKey(m_disp.is_keySPACE(), KeyButton::Space);
             handleKey(m_disp.is_keyBACKSPACE(), KeyButton::BackSpace);
             handleKey(m_disp.is_key1(), KeyButton::One);
             handleKey(m_disp.is_key2(), KeyButton::Two);
             handleKey(m_disp.is_key3(), KeyButton::Three);
             handleKey(m_disp.is_key4(), KeyButton::Four);
             handleKey(m_disp.is_key5(), KeyButton::Five);
             handleKey(m_disp.is_key6(), KeyButton::Six);
             handleKey(m_disp.is_key7(), KeyButton::Seven);
             handleKey(m_disp.is_key8(), KeyButton::Eight);
             handleKey(m_disp.is_key9(), KeyButton::Nine);
             handleKey(m_disp.is_keyPADADD(), KeyButton::Add);
             handleKey(m_disp.is_keyPADSUB(), KeyButton::Subtract);

             return true;
         }
         void getMousePos(ScreenPos &pos) const override final
         {
             pos.x = m_disp.mouse_x();
             pos.y = m_disp.mouse_y();
         }

         ModificationKey getModificationKeyMask() const override final
         {
             ModificationKey keyMask = ModificationKeyNone;
             if (m_disp.is_keySHIFTLEFT() || m_disp.is_keySHIFTRIGHT())
             {
                 keyMask = keyMask | ModificationKeyShift;
             }

             if (m_disp.is_keyCTRLLEFT() || m_disp.is_keyCTRLRIGHT())
             {
                 keyMask = keyMask | ModificationKeyCtrl;
             }

             if (m_disp.is_keyALT())
             {
                 keyMask = keyMask | ModificationKeyAlt;
             }

             return keyMask;

         }
         MouseButton getMouseButton() override final
         {
             unsigned int button = m_disp.button();
             MouseButton mouseButton = MouseButtonNone;
             if (button & 0x1)
             {
                 // Left mouse button
                 mouseButton = MouseButtonLeft;
             }
             else if (button & 0x2)
             {
                 // Right mouse button
                 mouseButton = MouseButtonRight;
             }
             else if (button & 0x4)
             {
                 // Middle mouse button
                 mouseButton = MouseButtonMiddle;
             }

             return mouseButton;
         }

         void* getWindow() override final
         {
             return (void*)&m_disp;
         }
     private:
         CImgDisplay& m_disp;
 };
 typedef std::shared_ptr<CImgMapControl> CImgMapControlPtr;


 int main()
 {
     // Set up window/display
     BlueMarble::MapPtr map = std::make_shared<Map>();
     int normalization = 0;
     CImgDisplay display(cimg_library::CImg<unsigned char>(), "BlueMarbleMaps Demo", normalization, true, true); //*static_cast<CImgDisplay*>(map->drawable()->getDisplay());
     display.resize(500, 500, true);

     // Test RasterVisualizer
     //auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/NE1_50M_SR_W/NE1_50M_SR_W.tif");
     //auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/nasa/eo_base_2020_clean_geo.tif");
     // auto backgroundDataSet = std::make_shared<BlueMarble::ImageDataSet>("/home/joar/git-repos/BlueMarbleMaps/geodata/blue_marble_256.jpg");
     // backgroundDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
     // auto backgroundLayer = BlueMarble::Layer(false);
     // auto rasterVis1 = std::make_shared<RasterVisualizer>();
     // rasterVis1->alpha(DirectDoubleAttributeVariable(1.0));
     // backgroundLayer.visualizers().push_back(rasterVis1);
     // backgroundLayer.addUpdateHandler(backgroundDataSet.get());
     // map->addLayer(&backgroundLayer);

     auto elevationDataSet = std::make_shared<BlueMarble::ImageDataSet>("C:/Users/Ottop/OneDrive/Skrivbord/goat.jpg");
     elevationDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
     auto elevationLayer = BlueMarble::Layer(false);
     elevationLayer.addUpdateHandler(elevationDataSet.get());
     auto rasterVis = std::make_shared<RasterVisualizer>();
     rasterVis->alpha(DirectDoubleAttributeVariable(0.5));
     elevationLayer.visualizers().push_back(rasterVis);
     map->addLayer(&elevationLayer);

     // Test Polygon/Line/Symbol visualizers
     auto vectorDataSet = std::make_shared<BlueMarble::MemoryDataSet>();
     vectorDataSet->initialize(BlueMarble::DataSetInitializationType::RightHereRightNow);
     auto vectorLayer = BlueMarble::Layer(true);
     vectorLayer.addUpdateHandler(vectorDataSet.get());
    
     std::vector<Point> points({{16, 56}, {17, 57}, {15, 58}});
     auto poly = std::make_shared<PolygonGeometry>(points);
     vectorDataSet->addFeature(vectorDataSet->createFeature(poly));

     map->addLayer(&vectorLayer);

     // Stress test by adding more layers and visualization using this
     //configureMap(map); 

     // Setup MapControl and event handlers
     CImgMapControlPtr mapControl = std::make_shared<CImgMapControl>(display);
     auto panHandler = BlueMarble::PanEventHandler(*map, mapControl);
     mapControl->addSubscriber(&panHandler);
     mapControl->setView(map);
    
     // Main loop
     //map->startInitialAnimation();
     map->update();
     while (!display.is_closed() && !display.is_keyESC()) 
     {
         mapControl->captureEvents();
         if (display.is_resized() || display.is_keyF11())
         {
             display.resize(false);
         }


         if (mapControl->updateRequired())
         {
             mapControl->updateViewInternal();
         }
         else
         {
             display.wait();
         }
     }
    
     return 0;
 }
 */