#include "BlueMarbleMaps/Core/Feature.h"


using namespace BlueMarble;

Feature::Feature(const Id& id, const CrsPtr& crs, const GeometryPtr& geometry)
    : m_id(id)
    , m_geometry(geometry)
    , m_attributes()
    , m_crs(crs)
{
}

Feature::Feature(const Id& id, const CrsPtr& crs, const GeometryPtr& geometry, const Attributes &attributes)
    : m_id(id)
    , m_geometry(geometry)
    , m_attributes(attributes)
    , m_crs(crs)
{
}

Feature::~Feature()
{
    
}

FeaturePtr Feature::clone()
{
    const GeometryPtr& geom = std::static_pointer_cast<Geometry>(m_geometry->clone());
    return std::make_shared<Feature>(m_id, m_crs, geom, m_attributes);
}

Id Feature::id() const
{
    return m_id;
}

void Feature::move(const Point &delta)
{
    m_geometry->move(delta);
}

void Feature::moveTo(const Point &point)
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

void Feature::reProjectTo(const CrsPtr &crs)
{
    if (geometryType() == GeometryType::Raster)
    {
        auto rasterGeom = geometryAsRaster();
        auto& currRaster = geometryAsRaster()->raster();

        auto newBounds = this->crs()->projectTo(crs, bounds());
        RasterGeometryPtr newRaster = std::make_shared<RasterGeometry>(currRaster, newBounds);

        for (int i(0); i < currRaster.width(); ++i)
        {
            for (int j(0); j < currRaster.height(); ++j)
            {
                auto p = newRaster->rasterIndexToPoint(i,j);
                auto pOld = crs->projectTo(m_crs, p);
                auto ind = rasterGeom->pointToRasterIndex(pOld);
                auto color = rasterGeom->raster().getColorAt(ind.x(), ind.y());
                newRaster->raster().setColorAt(i,j, color);
            }
        }
        m_geometry = newRaster;
    }
    else
    {
        m_geometry->forEachPoint([&](Point& p)
        {
            p = m_crs->projectTo(crs, p);
        });
    }

    m_crs = crs;
}

FeatureCollectionPtr Feature::projectTo(const CrsPtr &crs)
{
    FeatureCollectionPtr features = std::make_shared<FeatureCollection>();

    // if (geometryType() == GeometryType::Raster)
    // {
    //     // TODO: Raster Unsupported for now
    //     features->add(shared_from_this());
    //     return features;
    // }
    auto newFeature = clone();
    newFeature->reProjectTo(crs);
    features->add(newFeature);

    return features;
}

bool Feature::isInside(const Rectangle &bounds) const
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
