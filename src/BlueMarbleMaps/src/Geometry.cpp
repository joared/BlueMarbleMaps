#include "Core/Geometry.h"

using namespace BlueMarble;

Geometry::Geometry()
    : EngineObject()
{
    
}

PointGeometry::PointGeometry()
    : Geometry()
    , m_point()
{
}

PointGeometry::PointGeometry(const Point& point)
    : m_point(point)
{
}

LineGeometry::LineGeometry()
    : Geometry()
    , m_points()
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
    : Geometry()
    , m_rings()
    , m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
}

PolygonGeometry::PolygonGeometry(const PolygonGeometry &other)
    : Geometry()
    , m_rings(other.m_rings)
    , m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
}

PolygonGeometry::PolygonGeometry(const std::vector<Point>& ring)
    : Geometry()
    , m_cachedBounds([&] () { return Rectangle::fromPoints(outerRing());})
{
    m_rings.push_back(ring);
}

PolygonGeometry::PolygonGeometry(const std::vector<std::vector<Point>>& rings)
    : Geometry()
    , m_rings(rings)
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
    : Geometry()
    , m_raster()
{
}

BlueMarble::RasterGeometry::RasterGeometry(const Raster& raster, const Rectangle& bounds, double cellWidth, double cellHeight)
    : m_raster(raster)
    , m_bounds(bounds)
    , m_cellWidth(cellWidth)
    , m_cellHeight(cellHeight)
{
}

RasterGeometryPtr RasterGeometry::getSubRasterGeometry(const Rectangle& subBounds)
{
    if (!subBounds.overlap(bounds()))
    {
        std::cout << "RasterGeometry::getSubRasterGeometry() Bounds do not overlap: " << bounds().toString() << "(this), " << subBounds.toString() << " (other)\n";
        throw std::exception();
    }

    double cellWidth = m_cellWidth;
    double cellHeight = m_cellHeight;
    auto& raster = m_raster;

    // Retrie the crop that is inside the update area
    int x0 = std::max((int)(subBounds.xMin()/cellWidth), 0);
    int y0 = std::max((int)(subBounds.yMin()/cellHeight), 0);
    int x1 = std::min((int)(subBounds.xMax()/cellWidth), (int)(raster.width()-1));
    int y1 = std::min((int)(subBounds.yMax()/cellHeight), (int)(raster.height()-1));

    // New bounds of sub/cropped raster. Assuming 0 at the very left pixel
    Rectangle rasterBounds(x0*cellWidth, 
                           y0*cellHeight, 
                           (x1+1)*cellWidth, 
                           (y1+1)*cellHeight);

    // Create new sub geometry
    // auto subGeometry =  std::make_shared<RasterGeometry>();
    // subGeometry->m_cellWidth = cellWidth;
    // subGeometry->m_cellHeight = cellHeight;
    // subGeometry->m_bounds = rasterBounds;
    // subGeometry->m_raster = raster.getCrop(minPixelX, minPixelY, maxPixelX, maxPixelY);

    // return subGeometry;
    auto cropped = raster.getCrop(x0, y0, x1, y1);
    return std::make_shared<RasterGeometry>(cropped, rasterBounds, cellWidth, cellHeight);
}

MultiPolygonGeometry::MultiPolygonGeometry()
    :Geometry()
{
}

MultiPolygonGeometry::MultiPolygonGeometry(const std::vector<PolygonGeometry>& polygons)
    : Geometry()
    , m_polygons(polygons)
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
    return nullptr;
}

void BlueMarble::convertGeometry(GeometryPtr from, GeometryType toType)
{

}
