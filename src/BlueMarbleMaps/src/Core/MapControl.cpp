#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/SoftwareDrawable.h"
#include "BlueMarbleMaps/Core/OpenGLDrawable.h"
#include <glfw3.h>
#include <exception>
#include <cstring>

using namespace BlueMarble;

MapControl::MapControl()
    : m_mapView(nullptr)
    , m_tool(nullptr)
    , m_updateRequired(false)
{

}

void MapControl::setView(MapPtr mapView)
{
    // First disconnect tool
    auto tool = m_tool;
    if (tool)
    {
        setTool(nullptr);
    }

    if (m_mapView)
    {
        std::cout << "MapControl::setView() Detaching view, setting Bitmap drawable\n";
        auto drawable = std::make_shared<SoftwareBitmapDrawable>(500, 500, 4);
        m_mapView->drawable(drawable);
        m_mapView->onDetachedFromMapControl();
    }

    if (mapView)
    {
        if (auto window = getWindow())
        {
            std::cout << "MapControl::setView() Attaching Window drawable\n";
        
            //Create Opengl Drawable
            auto drawable = std::make_shared<WindowOpenGLDrawable>(1000, 1000, 4);
            drawable->setWindow(window);
            mapView->drawable(drawable);
            mapView->onAttachedToMapControl(shared_from_this());
        }
        else
        {
            std::cout << "No window provided in MapControl::getWindow()\n";
            throw std::exception();
        }
    }

    // set the view
    m_mapView = mapView;

    // Reinitialize whatever tool we have
    if (tool)
    {
        setTool(tool);
    }
}

MapPtr MapControl::getView()
{
    return m_mapView;
}

void MapControl::setTool(const ToolPtr &tool)
{
    if (m_mapView == nullptr)
    {
        // For now, we are not allowed to attach a tool without a map view
        BMM_DEBUG() << "MapControl::setTool() called when no map view is attached\n";
        throw std::exception();
    }

    if (m_tool)
    {
        removeSubscriber(m_tool.get());
        m_tool->onDisconnectedFromMapControl();      
    }

    m_tool = tool;
    if (m_tool)
    {
        addSubscriber(m_tool.get());
        m_tool->onConnectedToMapControl(shared_from_this(), m_mapView);
    }
}

void MapControl::updateView()
{
    // TODO: scheduling
    m_updateRequired = true;
}

void MapControl::updateViewInternal()
{
    if (m_updateRequired)
    {
        m_updateRequired = false;
        bool updateRequired = m_mapView->update(true);
        m_updateRequired = m_updateRequired || updateRequired; // FIXME: Uggly fix to enable calling update() inside MapEvents during an update
    }
}

bool MapControl::updateRequired()
{
    return m_updateRequired;
}

bool MapControl::resize(int width, int height, int64_t timeStampMs)
{
    handleResize(width, height);
    return EventManager::resize(width, height, timeStampMs);
}

void MapControl::setMouseCursor(MouseCursor cursor)
{
    // For more cursors: https://www.glfw.org/docs/3.3/group__shapes.html#gabb3eb0109f11bb808fc34659177ca962
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(getWindow());
    if (!window)
    {
        throw std::runtime_error("MapControl::setMouseCursor() Failed to retrieve glfw window");
    }
    
    GLFWcursor* glfwCursor = nullptr;
    switch (cursor)
    {
    case MouseCursor::Standard:
        glfwCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        break;
    case MouseCursor::Custom:
        unsigned char pixels[16 * 16 * 4];
        memset(pixels, 0xff, sizeof(pixels));
        GLFWimage image;
        image.width = 16;
        image.height = 16;
        image.pixels = pixels;
        glfwCursor = glfwCreateCursor(&image, 0, 0);
    default:
        break;
    }
    

    glfwSetCursor(window, glfwCursor);
}

void MapControl::handleResize(int width, int height)
{
    double prevMapWidth = m_mapView->width();
    m_mapView->drawable()->resize(width, height);
    // TODO: Use options for resizing
    // 1. Keep center and map width
    m_mapView->scale(width / prevMapWidth);
    // 2. Keep center and scale (i.e do nothing?)
    // ...
    // 3. Something else?
    
    
    updateView();
    updateViewInternal();
}
