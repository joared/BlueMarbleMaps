#include "Feature.h"

using namespace BlueMarble;

Feature::Feature(const Id& id, GeometryPtr geometry)
    : m_id(id)
    , m_geometry(geometry)
    , m_attributes()
{
}

BlueMarble::Feature::Feature(const Id &id, GeometryPtr geometry, const Attributes &attributes)
    : m_id(id)
    , m_geometry(geometry)
    , m_attributes(attributes)
{
}

FeaturePtr BlueMarble::Feature::clone()
{
    return std::make_shared<Feature>(id(), m_geometry->clone(), attributes());
}

Id Feature::id() const
{
    return m_id;
}

void BlueMarble::Feature::move(const Point &delta)
{
    m_geometry->move(delta);
}

void BlueMarble::Feature::moveTo(const Point &point)
{
    m_geometry->moveTo(point);
}

Rectangle Feature::bounds() const
{
    return m_geometry->calculateBounds();
}

Point Feature::center() const
{
    return m_geometry->center();
}

bool Feature::isInside(const Rectangle& bounds) const
{
    return m_geometry->isInside(bounds);
}

bool Feature::isStrictlyInside(const Rectangle &bounds) const
{
    return m_geometry->isStrictlyInside(bounds);
}

GeometryType Feature::geometryType() const
{
    return m_geometry->type();
}

GeometryPtr& Feature::geometry()
{
    return m_geometry;
}

PointGeometryPtr Feature::geometryAsPoint()
{
    if (m_geometry->type() == GeometryType::Point)
        return std::static_pointer_cast<PointGeometry>(m_geometry);
    
    return nullptr;
}

LineGeometryPtr Feature::geometryAsLine()
{
    if (m_geometry->type() == GeometryType::Line)
        return std::static_pointer_cast<LineGeometry>(m_geometry);
    
    return nullptr;
}

PolygonGeometryPtr Feature::geometryAsPolygon()
{
    if (m_geometry->type() == GeometryType::Polygon)
        return std::static_pointer_cast<PolygonGeometry>(m_geometry);
    
    return nullptr;
}

MultiPolygonGeometryPtr BlueMarble::Feature::geometryAsMultiPolygon()
{
    if (m_geometry->type() == GeometryType::MultiPolygon)
        return std::static_pointer_cast<MultiPolygonGeometry>(m_geometry);
    
    return nullptr;
}

RasterGeometryPtr Feature::geometryAsRaster()
{
    if (m_geometry->type() == GeometryType::Raster)
        return std::static_pointer_cast<RasterGeometry>(m_geometry);
    
    return nullptr;
}

Attributes& Feature::attributes()
{
    return m_attributes;
}

std::string BlueMarble::Feature::prettyString()
{
    std::string s = "Feature (" + std::to_string(m_id.dataSetId()) + ", " + std::to_string(m_id.featureId()) + "):\n";
    s += "\tGeometry: " + typeToString(m_geometry->type());
    s += "\n";
    s += "\tAttributes:\n";
    
    for (auto attr : m_attributes)
    {
        s += "\t\t" + attr.first;
        s += " : " + attributeToString(attr.second);
        s += "\n";
    }

    return s;
}
