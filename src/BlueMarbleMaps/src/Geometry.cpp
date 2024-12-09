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

void LineGeometry::move(const Point& delta)
{
    Utils::movePoints(m_points, delta);
}

void LineGeometry::moveTo(const Point& point)
{
    Utils::movePointsTo(m_points, point);
}

PolygonGeometry::PolygonGeometry()
    : m_rings()
    , m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
}

PolygonGeometry::PolygonGeometry(const PolygonGeometry &other)
    : m_rings(other.m_rings)
    , m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
}

PolygonGeometry::PolygonGeometry(const std::vector<Point>& ring)
    : m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
    m_rings.push_back(ring);
}

PolygonGeometry::PolygonGeometry(const std::vector<std::vector<Point>>& rings)
    : m_rings(rings)
    , m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
}

PolygonGeometry& PolygonGeometry::operator=(const PolygonGeometry& other)
{
    m_rings = other.m_rings;
    m_cachedBounds = Utils::CachableVariable<Rectangle>([&] () { return Rectangle::fromPoints(outerRing());});

    return *this;
}

void PolygonGeometry::move(const Point& delta)
{
    for (auto& ring : m_rings)
    {
        Utils::movePoints(ring, delta);
    }
}

void PolygonGeometry::moveTo(const Point& point)
{
    for (auto& ring : m_rings)
    {
        Utils::movePointsTo(ring, point);
    }
}

RasterGeometry::RasterGeometry()
    : m_raster()
{
}

BlueMarble::RasterGeometry::RasterGeometry(const Raster& raster, const Rectangle& bounds, double cellWidth, double cellHeight)
    : m_raster(raster)
    , m_bounds(bounds)
    , m_cellWidth(cellWidth)
    , m_cellHeight(cellHeight)
{
}

MultiPolygonGeometry::MultiPolygonGeometry()
{
}

MultiPolygonGeometry::MultiPolygonGeometry(const std::vector<PolygonGeometry>& polygons)
    : m_polygons(polygons)
{
}

void MultiPolygonGeometry::move(const Point& delta)
{
    for (auto& pol : m_polygons)
    {
        pol.move(delta);
    }
}

void MultiPolygonGeometry::moveTo(const Point& point)
{
    for (auto& pol : m_polygons)
    {
        pol.moveTo(point);
    }
}

PointGeometryPtr polygonToPoint(PolygonGeometryPtr polygon)
{

}

void BlueMarble::convertGeometry(GeometryPtr from, GeometryType toType)
{

}
