#include "BlueMarbleMaps/CoordinateSystem/Crs.h"

using namespace BlueMarble;

CrsPtr Crs::wgs84LngLat()
{
    return std::make_shared<Crs>(
        GeodeticDatum::wgs84(),
        Projection::longLat()
    );
}


Crs::Crs(const GeodeticDatumPtr& datum, const ProjectionPtr& projection)
    : m_datum(datum)
    , m_projection(projection)
{
}


Point Crs::projectTo(const CrsPtr& crs, const Point& point)
{
    const auto& ellipsoid = m_datum->ellipsoid();
    auto lngLat = m_projection->unProject(point, ellipsoid);
    return crs->projection()->project(lngLat, ellipsoid);
}


Rectangle Crs::projectTo(const CrsPtr& crs, const Rectangle& rect)
{
    std::vector<Point> newCorners;
    for (const auto& p : rect.corners())
    {
        newCorners.emplace_back(projectTo(crs, p));
    }

    return Rectangle::fromPoints(newCorners);
}
