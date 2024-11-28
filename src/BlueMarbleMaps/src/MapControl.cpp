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
    auto& drawable = m_mapView->drawable();
    auto window = getWindow();
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
