#include "Core/MapControl.h"
#include "Core/SoftwareDrawable.h"
#include "Core/OpenGLDrawable.h"
#include <exception>

using namespace BlueMarble;

MapControl::MapControl()
    : m_mapView(nullptr)
    , m_updateRequired(false)
{

}

void MapControl::setView(MapPtr mapView)
{
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
}

MapPtr MapControl::getView()
{
    return m_mapView;
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
    m_mapView->drawable()->resize(width, height);
    updateView();
    updateViewInternal();
}
