#include "BlueMarbleMaps/Core/Feature.h"

using namespace BlueMarble;

Feature::Feature(const Id& id, const CrsPtr& crs, const GeometryPtr& geometry)
    : m_id(id)
    , m_geometry(geometry)
    , m_attributes()
    , m_crs(crs)
{
}

BlueMarble::Feature::Feature(const Id& id, const CrsPtr& crs, const GeometryPtr& geometry, const Attributes &attributes)
    : m_id(id)
    , m_geometry(geometry)
    , m_attributes(attributes)
    , m_crs(crs)
{
}

FeaturePtr BlueMarble::Feature::clone()
{
    const GeometryPtr& geom = std::static_pointer_cast<Geometry>(m_geometry->clone());
    return std::make_shared<Feature>(m_id, m_crs, geom, m_attributes);
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

FeatureCollectionPtr BlueMarble::Feature::projectTo(const CrsPtr &crs)
{
    // TODO
    return FeatureCollectionPtr();
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

const GeometryPtr& Feature::geometry() const
{
    return m_geometry;
}

PointGeometryPtr Feature::geometryAsPoint() const
{
    if (m_geometry->type() == GeometryType::Point)
        return std::static_pointer_cast<PointGeometry>(m_geometry);
    
    return nullptr;
}

LineGeometryPtr Feature::geometryAsLine() const
{
    if (m_geometry->type() == GeometryType::Line)
        return std::static_pointer_cast<LineGeometry>(m_geometry);
    
    return LineGeometryPtr();
}

PolygonGeometryPtr Feature::geometryAsPolygon() const
{
    if (m_geometry->type() == GeometryType::Polygon)
        return std::static_pointer_cast<PolygonGeometry>(m_geometry);
    
    return nullptr;
}

MultiPolygonGeometryPtr BlueMarble::Feature::geometryAsMultiPolygon() const
{
    if (m_geometry->type() == GeometryType::MultiPolygon)
        return std::static_pointer_cast<MultiPolygonGeometry>(m_geometry);
    
    return nullptr;
}

RasterGeometryPtr Feature::geometryAsRaster() const
{
    if (m_geometry->type() == GeometryType::Raster)
        return std::static_pointer_cast<RasterGeometry>(m_geometry);
    
    return nullptr;
}

Attributes& Feature::attributes()
{
    return m_attributes;
}

std::string BlueMarble::Feature::prettyString() const
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
