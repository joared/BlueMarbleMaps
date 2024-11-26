#include "MapConstraints.h"
#include "Map.h"
#include "Utils.h"

using namespace BlueMarble;


void MapConstraints::constrainMap(Map &map)
{
    // Constrain center
    if (!m_bounds.isInside(map.center()))
    {
        auto center = map.center();
        map.center
        (
            Point
            (
                constrainValue(center.x(), m_bounds.xMin(), m_bounds.xMax()),
                constrainValue(center.y(), m_bounds.yMin(), m_bounds.yMax())
            )
        );
    }
    // Constrain scale
    map.scale(constrainValue(map.scale(), m_minScale, m_maxScale));
}

double MapConstraints::constrainValue(double val, double minVal, double maxVal)
{
    return Utils::clampValue(val, minVal, maxVal);
}
