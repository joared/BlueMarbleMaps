#include "Feature.h"

using namespace BlueMarble;

Feature::Feature(const Id& id, GeometryPtr geometry)
    : m_id(id)
    , m_geometry(geometry)
{
}

Id Feature::id() const
{
    return m_id;
}

Rectangle Feature::bounds() const
{
    return m_geometry->bounds();
}

Point Feature::center() const
{
    return m_geometry->center();
}

bool BlueMarble::Feature::isInside(const Rectangle& bounds) const
{
    return m_geometry->isInside(bounds);
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
