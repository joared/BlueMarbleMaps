#include "BlueMarbleMaps/Core/PresentationObject.h"
#include "BlueMarbleMaps/Utility/Utils.h"
#include "BlueMarbleMaps/Core/Map.h"

using namespace BlueMarble;

PresentationObject::PresentationObject(FeaturePtr feature, 
                                       FeaturePtr sourceFeature, 
                                       Visualizer* visualizer,
                                       bool isSelected,
                                       int nodeIndex)
    : m_feature(feature)
    , m_sourceFeature(sourceFeature)
    , m_visualizer(visualizer)
    , m_isSelected(isSelected)
    , m_nodeIndex(nodeIndex)
    , m_children() // TODO
{
}

bool PresentationObject::hitTest(const Map& map, int x, int y, double pointerRadius)
{
    // std::cout << "PresentationObject::hitTest\n";
    auto mapPoint = map.screenToMap(Point(x,y));
    pointerRadius /= map.scale();

    auto f = m_feature;
    // TODO: delagate hittesting to visualizer. We don't know line width etc.
    switch (f->geometryType())
    {
        case GeometryType::Point:
            return hitTestPoint(mapPoint.x(), mapPoint.y(), pointerRadius, f->geometryAsPoint());
        case GeometryType::Line:
            return hitTestLine(mapPoint.x(), mapPoint.y(), pointerRadius, f->geometryAsLine());
        case GeometryType::Polygon:
            return hitTestPolygon(mapPoint.x(), mapPoint.y(), pointerRadius, f->geometryAsPolygon());
        case GeometryType::Raster:
            return hitTestRaster(mapPoint.x(), mapPoint.y(), pointerRadius, f->geometryAsRaster());
        default:
            std::cout << "PresentationObject::hitTest() Unhandled geometry type: " << (int)f->geometryType() << "\n";
    }

    return false;
}

bool PresentationObject::hitTest(const Rectangle& bounds)
{
    // std::cout << "PresentationObject::hitTest\n";
    return m_feature->isInside(bounds);
}

int PresentationObject::nodeIndex() const
{
    return m_nodeIndex;
}
bool PresentationObject::isSelected() const
{
    return m_isSelected;
}

bool PresentationObject::equals(const PresentationObject& other)
{
    // This makes it possible to perform "partial" selection of 
    // a feature by selecting a presentation object
    return (m_visualizer->visualizerType() == other.visualizer()->visualizerType() &&
            m_feature->id() == other.feature()->id() &&
            m_feature->geometryType() == other.feature()->geometryType() &&
            m_nodeIndex == other.nodeIndex());
}

bool BlueMarble::hitTestPoint(double x, double y, double pointerRadius, PointGeometryPtr geometry)
{
    // std::cout << "hitTestPoint\n";
    auto& point = geometry->point();
    return (Point(x,y)-point).length() < pointerRadius;
}

bool BlueMarble::hitTestLine(double x, double y, double pointerRadius, LineGeometryPtr geometry)
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

bool BlueMarble::hitTestPolygon(double x, double y, double pointerRadius, PolygonGeometryPtr geometry)
{
    // std::cout << "hitTestPolygon\n";
    // TOOD: take pointerRadius into account

    // First check if the point is inside the outer ring
    auto polygon = geometry->outerRing();
    if (!Utils::pointInsidePolygon(Point(x,y), polygon)) return false;

    // Then check that the point is not inside any of the inner rings
    // for (size_t i=1; i<geometry->rings().size(); i++)
    // {
    //     if (Utils::pointInsidePolygon(Point(x,y), geometry->rings()[i]))
    //         return false;
    // }

    return true;
}

bool BlueMarble::hitTestRaster(double x, double y, double pointerRadius, RasterGeometryPtr geometry)
{
    // std::cout << "hitTestRaster\n";
    return false;
}
