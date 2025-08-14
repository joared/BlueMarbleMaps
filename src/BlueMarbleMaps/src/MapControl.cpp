#include "BlueMarbleMaps/Core/MapControl.h"
#include "BlueMarbleMaps/Core/SoftwareDrawable.h"
#include "BlueMarbleMaps/Core/OpenGLDrawable.h"
#include <exception>

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

    if (!mapView)
    {
        std::cout << "MapControl::setView() Detaching view, setting Bitmap drawable\n";
        auto drawable = std::make_shared<SoftwareBitmapDrawable>(500, 500, 4);
        mapView->drawable(drawable);
        m_mapView = mapView;
        m_mapView->onDetachedFromMapControl();
    }
    else if (auto window = getWindow())
    {
        std::cout << "MapControl::setView() Attaching Window drawable\n";
        
        //Create Opengl Drawable
        auto drawable = std::make_shared<WindowOpenGLDrawable>(1000, 1000, 4);
        drawable->setWindow(window);
        mapView->drawable(drawable);
        m_mapView = mapView;
        m_mapView->onAttachedToMapControl(this);
    }
    else
    {
        std::cout << "No window provided in MapControl::getWindow()\n";
        throw std::exception();
    }

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
        m_tool->onDisconnected();      
    }

    m_tool = tool;
    if (m_tool)
    {
        addSubscriber(m_tool.get());
        m_tool->onConnected(shared_from_this(), m_mapView);
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
        m_updateRequired = m_mapView->update(true);
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
