#include "BlueMarbleMaps/Core/Geometry.h"


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
    , m_isClosed(false)
{
}

LineGeometry::LineGeometry(const std::vector<Point>& points)
    : Geometry()
    , m_points(points)
    , m_isClosed(false)
{
}

BlueMarble::LineGeometry::LineGeometry(const Rectangle &rect)
    : Geometry()
    , m_points(rect.corners())
    , m_isClosed(true)
{
}

double LineGeometry::length() const
{
    double length = 0.0;
    for (int i(0); i < (int)(m_points.size()-1); ++i)
    {
        const auto& p1 = m_points[i];
        const auto& p2 = m_points[i+1];
        length += (p2-p1).length();
    }

    return length;
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

Rectangle BlueMarble::PolygonGeometry::calculateBounds()
{
    //return m_cachedBounds.get(); // FIXME: this is quicker, but we need to ensure that we update it when the geometry changes
    return Rectangle::fromPoints(outerRing()); //{  };
}
void PolygonGeometry::move(const Point &delta)
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
    BMM_DEBUG() << "UNINITIALIZED RASTER GEOMETRY!\n";
    throw std::runtime_error("UNINITIALIZED RASTER GEOMETRY!");
}

BlueMarble::RasterGeometry::RasterGeometry(const Raster& raster, const Rectangle& bounds)
    : Geometry()
    , m_raster(raster)
    , m_bounds(bounds)
{
    if (cellWidth() <= 0 || cellHeight() <= 0)
    {
        throw std::runtime_error("Invalid cell size: " + std::to_string(cellWidth()) + "," + std::to_string(cellHeight()));
    }
}

Point RasterGeometry::pointToRasterIndex(const Point& point) const
{
    if (!m_bounds.isInside(point))
        return Point::undefined();

    double xRel = point.x() - m_bounds.xMin();
    double yRel = m_bounds.yMax() - point.y();

    int xInd = (int)std::round(xRel / cellWidth());
    int yInd = (int)std::round(yRel / cellHeight());

    return Point(xInd, yInd);
}

Point BlueMarble::RasterGeometry::rasterIndexToPoint(int x, int y) const
{
    //assert(x < raster().width());
    //assert(y < raster().height());

    return Point(bounds().xMin() + cellWidth()*x, bounds().yMax() - cellHeight()*y);
}

RasterGeometryPtr RasterGeometry::getSubRasterGeometry(const Rectangle &subBounds)
{
    if (!subBounds.overlap(bounds()))
    {
        std::cout << "RasterGeometry::getSubRasterGeometry() Bounds do not overlap: " << bounds().toString() << "(this), " << subBounds.toString() << " (other)\n";
        throw std::exception();
    }

    double cellW = cellWidth();
    double cellH = cellHeight();
    auto& raster = m_raster;

    // Retrie the crop that is inside the update area
    int x0 = std::max((int)(subBounds.xMin()/cellW), 0);
    int y0 = std::max((int)(subBounds.yMin()/cellH), 0);
    int x1 = std::min((int)(subBounds.xMax()/cellW), (int)(raster.width()-1));
    int y1 = std::min((int)(subBounds.yMax()/cellH), (int)(raster.height()-1));

    // New bounds of sub/cropped raster. Assuming 0 at the very left pixel
    Rectangle rasterBounds(x0*cellW, 
                           y0*cellH, 
                           (x1+1)*cellW, 
                           (y1+1)*cellH);

    // Create new sub geometry
    // auto subGeometry =  std::make_shared<RasterGeometry>();
    // subGeometry->m_cellWidth = cellWidth;
    // subGeometry->m_cellHeight = cellHeight;
    // subGeometry->m_bounds = rasterBounds;
    // subGeometry->m_raster = raster.getCrop(minPixelX, minPixelY, maxPixelX, maxPixelY);

    // return subGeometry;
    auto cropped = raster.getCrop(x0, y0, x1, y1);
    return std::make_shared<RasterGeometry>(cropped, rasterBounds);
}

MultiPolygonGeometry::MultiPolygonGeometry()
    : Geometry()
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
