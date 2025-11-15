#ifndef BLUEMARBLE_FEATURE
#define BLUEMARBLE_FEATURE

#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Geometry.h"
#include "BlueMarbleMaps/Core/Identity.h"
#include "BlueMarbleMaps/Core/Attributes.h"
#include "BlueMarbleMaps/Core/Collection.h"
#include "BlueMarbleMaps/CoordinateSystem/Crs.h"

#include <memory>

namespace BlueMarble
{
    // Forward declaration
    class Feature;
    typedef std::shared_ptr<Feature> FeaturePtr;

    typedef Collection<FeatureId>                FeatureIdCollection;
    typedef std::shared_ptr<FeatureIdCollection> FeatureIdCollectionPtr;

    typedef Collection<Id>                IdCollection;
    typedef std::shared_ptr<IdCollection> IdCollectionPtr;

    class FeatureCollection;
    typedef std::shared_ptr<FeatureCollection> FeatureCollectionPtr;

    class Feature 
        : public std::enable_shared_from_this<Feature> // This inheritance is only for projectTo for unssuported raster geometries and should be removed later
    {
        public:
            Feature(const Id& id, const CrsPtr& crs, const GeometryPtr& geometry);
            Feature(const Id& id, const CrsPtr& crs, const GeometryPtr& geometry, const Attributes& attributes);
            FeaturePtr clone();
            Id id() const;
            void id(const Id& id) { m_id = id; }; // TODO: remove, temporary fix for setting id
            const CrsPtr& crs() { return m_crs; }
            void move(const Point& delta);
            void moveTo(const Point& point);
            Rectangle bounds() const;
            Point center() const;
            FeatureCollectionPtr projectTo(const CrsPtr& crs);
            bool isInside(const Rectangle& bounds) const; // TODO: change name to overlap?
            bool isStrictlyInside(const Rectangle& bounds) const;
            GeometryType geometryType() const;
            const GeometryPtr& geometry() const;
            PointGeometryPtr geometryAsPoint() const;
            LineGeometryPtr geometryAsLine() const;
            PolygonGeometryPtr geometryAsPolygon() const;
            MultiPolygonGeometryPtr geometryAsMultiPolygon() const;
            RasterGeometryPtr geometryAsRaster() const;
            Attributes& attributes();
            std::string prettyString() const;
        private:
            Feature(const Feature&) = default; // Make copy constructor private. Call clone() to copy.
            void reProjectTo(const CrsPtr& crs); // Modifies this feature and its geometry. Keep private for now.

            Id          m_id;
            GeometryPtr m_geometry;
            Attributes  m_attributes;
            CrsPtr      m_crs;
    };

    class FeatureCollection : public Collection<FeaturePtr>
    {
    public:
        using Base = Collection<FeaturePtr>;
        using Base::Base; // inherit constructors
        
        using Base::remove; // Keep base remove (hidden otherwise due to overload)
        void remove(const Id& id)
        {
            for (size_t i(0); i<size(); ++i)
            {
                if (get(i)->id() == id)
                {
                    Base::remove(i);
                    return;
                }
            }

            std::cout << "FeatureCollection::remove() Feature with id '(" << id.dataSetId() << ", " << id.featureId() << ")' doesn't exist!";
            throw std::exception();
        }

    };
}

#endif /* BLUEMARBLE_FEATURE */
