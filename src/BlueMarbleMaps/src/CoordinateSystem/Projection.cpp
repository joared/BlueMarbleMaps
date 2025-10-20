#include "BlueMarbleMaps/CoordinateSystem/Projection.h"

using namespace BlueMarble;

Projection::Projection()
{
}

ProjectionPtr Projection::longLat()
{
    return std::make_shared<LongLatProjection>();
}