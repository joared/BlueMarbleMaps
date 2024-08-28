#include "PresentationObject.h"
#include "Utils.h"

using namespace BlueMarble;

PresentationObject::PresentationObject(FeaturePtr feature, FeaturePtr sourceFeature)
    : m_feature(feature)
    , m_sourceFeature(sourceFeature)
{
}

bool PresentationObject::hitTest(int x, int y, double pointerRadius)
{
    // std::cout << "PresentationObject::hitTest\n";
    switch (m_feature->geometryType())
    {
        case GeometryType::Point:
            return hitTestPoint(x, y, pointerRadius, m_feature->geometryAsPoint()->point());
        case GeometryType::Line:
            return hitTestLine(x, y, pointerRadius, m_feature->geometryAsLine()->points());
        case GeometryType::Polygon:
            return hitTestPolygon(x, y, pointerRadius, m_feature->geometryAsPolygon()->points());
        case GeometryType::Raster:
            return hitTestRaster(x, y, pointerRadius, m_feature->geometryAsRaster()->raster());
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

bool BlueMarble::hitTestPoint(int x, int y, double pointerRadius, const Point &point)
{
    // std::cout << "hitTestPoint\n";
    return (Point(x,y)-point).length() < pointerRadius;
}

bool BlueMarble::hitTestLine(int x, int y, double pointerRadius, const std::vector<Point> &line)
{
    // std::cout << "hitTestLine\n";
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

bool BlueMarble::hitTestPolygon(int x, int y, double pointerRadius, const std::vector<Point> &polygon)
{
    // std::cout << "hitTestPolygon\n";
    // TOOD: take pointerRadius into account
    return Utils::pointInsidePolygon(Point(x,y), polygon);
}

bool BlueMarble::hitTestRaster(int x, int y, double pointerRadius, const Raster &raster)
{
    // std::cout << "hitTestRaster\n";
    return false;
}
