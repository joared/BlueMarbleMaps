#include "PresentationObject.h"
#include "Utils.h"

using namespace BlueMarble;

PresentationObject::PresentationObject(FeaturePtr feature, FeaturePtr sourceFeature, Visualizer* visualizer)
    : m_feature(feature)
    , m_sourceFeature(sourceFeature)
    , m_visualizer(visualizer)
{
}

bool PresentationObject::hitTest(int x, int y, double pointerRadius)
{
    // std::cout << "PresentationObject::hitTest\n";
    switch (m_feature->geometryType())
    {
        case GeometryType::Point:
            return hitTestPoint(x, y, pointerRadius, m_feature->geometryAsPoint());
        case GeometryType::Line:
            return hitTestLine(x, y, pointerRadius, m_feature->geometryAsLine());
        case GeometryType::Polygon:
            return hitTestPolygon(x, y, pointerRadius, m_feature->geometryAsPolygon());
        case GeometryType::Raster:
            return hitTestRaster(x, y, pointerRadius, m_feature->geometryAsRaster());
        default:
            std::cout << "PresentationObject::hitTest() Unhandled geometry type: " << (int)m_feature->geometryType() << "\n";
    }

    return false;
}

bool PresentationObject::hitTest(const Rectangle& bounds)
{
    // std::cout << "PresentationObject::hitTest\n";
    return m_feature->isInside(bounds);
}

bool BlueMarble::hitTestPoint(int x, int y, double pointerRadius, PointGeometryPtr geometry)
{
    // std::cout << "hitTestPoint\n";
    auto& point = geometry->point();
    return (Point(x,y)-point).length() < pointerRadius;
}

bool BlueMarble::hitTestLine(int x, int y, double pointerRadius, LineGeometryPtr geometry)
{
    // std::cout << "hitTestLine\n";
    auto& line = geometry->points();
    auto p = Point(x, y);
    for (int i(0); i < line.size()-1; i++)
    {
        if (Utils::distanceToLine(p, line[i], line[i+1]) < pointerRadius)
        {
            return true;
        }
    }

    return false;
}

bool BlueMarble::hitTestPolygon(int x, int y, double pointerRadius, PolygonGeometryPtr geometry)
{
    // std::cout << "hitTestPolygon\n";
    // TOOD: take pointerRadius into account

    // First check if the point is inside the outer ring
    auto& polygon = geometry->outerRing();
    if (!Utils::pointInsidePolygon(Point(x,y), polygon)) return false;

    // Then check that the point is not inside any of the inner rings
    for (size_t i=1; i<geometry->rings().size(); i++)
    {
        if (Utils::pointInsidePolygon(Point(x,y), geometry->rings()[i]))
            return false;
    }

    return true;
}

bool BlueMarble::hitTestRaster(int x, int y, double pointerRadius, RasterGeometryPtr geometry)
{
    // std::cout << "hitTestRaster\n";
    return false;
}
