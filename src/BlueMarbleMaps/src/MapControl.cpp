#include "MapControl.h"
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
        auto drawable = std::make_shared<BitmapDrawable>(500, 500, 3);
        mapView->drawable(drawable);
        m_mapView = mapView;
        m_mapView->onDetachedFromMapControl();
    }
    else if (auto window = getWindow())
    {
        std::cout << "MapControl::setView() Attaching Window drawable\n";
        auto drawable = std::make_shared<WindowDrawable>(500, 500, 3);
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

bool BlueMarble::MapControl::updateRequired()
{
    return m_updateRequired;
}

void BlueMarble::MapControl::handleResize(int width, int height)
{
    m_mapView->drawable()->resize(width, height);
}
