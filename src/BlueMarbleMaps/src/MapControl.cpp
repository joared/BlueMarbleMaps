#include "MapControl.h"

using namespace BlueMarble;

MapControl::MapControl()
    : m_mapView(nullptr)
    , m_updateRequired(false)
{

}

void MapControl::setView(MapPtr mapView)
{
    m_mapView = mapView;

    // TODO decide drawable and possibly set window handle
    if (auto window = getWindow())
    {
        std::cout << "MapControl::setView() Setting new drawable\n";
        auto drawable = std::make_shared<WindowDrawable>(500, 500, 3);
        drawable->setWindow(window);
        m_mapView->drawable(drawable);
    }
    //drawable.setWindowHandle(window)
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

void BlueMarble::MapControl::handleResize(int width, int height)
{
    m_mapView->drawable()->resize(width, height);
}
