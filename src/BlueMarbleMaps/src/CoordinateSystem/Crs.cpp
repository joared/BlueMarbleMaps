#include "CoordinateSystem/Crs.h"

using namespace BlueMarble;

CrsPtr Crs::wgs84LngLat()
{
    return std::make_shared<Crs>();
}

Crs::Crs()
{
}

void Crs::projectTo(const CrsPtr &crs, const Point &point)
{
}

void Crs::projectTo(const CrsPtr &crs, const Rectangle &rect)
{
}
