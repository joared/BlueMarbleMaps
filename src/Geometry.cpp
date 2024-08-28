#include "Geometry.h"

using namespace BlueMarble;

PointGeometry::PointGeometry()
    : m_point()
{
}

PointGeometry::PointGeometry(const Point& point)
    : m_point(point)
{
}

LineGeometry::LineGeometry()
    : m_points()
{
}

LineGeometry::LineGeometry(const std::vector<Point>& points)
    : m_points(points)
{
}

PolygonGeometry::PolygonGeometry()
    : m_points()
    , m_cachedBounds([&] () { return Rectangle::fromPoints(m_points);})
{
}

PolygonGeometry::PolygonGeometry(const PolygonGeometry &other)
    : m_points(other.m_points)
    , m_cachedBounds([&] () { return Rectangle::fromPoints(m_points);})
{
}

PolygonGeometry::PolygonGeometry(const std::vector<Point>& points)
    : m_points(points)
    , m_cachedBounds([&] () { return Rectangle::fromPoints(m_points);})
{
}

PolygonGeometry& PolygonGeometry::operator=(const PolygonGeometry& other)
{
    m_points = other.m_points;
    m_cachedBounds = Utils::CachableVariable<Rectangle>([&] () { return Rectangle::fromPoints(m_points);});

    return *this;
}

RasterGeometry::RasterGeometry()
    : m_raster()
    , m_offset()
{
}

RasterGeometry::RasterGeometry(const Raster& raster, const Point& offset)
    : m_raster(raster)
    , m_offset(offset)
{
}

MultiPolygonGeometry::MultiPolygonGeometry()
{
}

MultiPolygonGeometry::MultiPolygonGeometry(const std::vector<PolygonGeometry>& polygons)
    : m_polygons(polygons)
{
}
