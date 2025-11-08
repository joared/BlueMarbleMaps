#include "BlueMarbleMaps/Core/MapConstraints.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Utility/Utils.h"

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
    //map.scale(constrainValue(map.scale(), m_minScale, m_maxScale)); // TODO add back
}

double MapConstraints::constrainValue(double val, double minVal, double maxVal)
{
    return val;
    //return Utils::clampValue(val, minVal, maxVal); // TODO add back
}
