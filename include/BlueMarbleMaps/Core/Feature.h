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

    typedef Collection<FeatureId>              FeatureIdCollection;
    typedef std::shared_ptr<FeatureIdCollection> FeatureIdCollectionPtr;

    class FeatureCollection;
    typedef std::shared_ptr<FeatureCollection> FeatureCollectionPtr;

    class Feature
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

    
    // class FeatureCollection
    // {
    //     public:
    //         FeatureCollection()
    //             : m_features()
    //         {}
    //         void add(FeaturePtr feature)
    //         {
    //             m_features.push_back(feature);
    //         }

    //         void remove(const Id& id)
    //         {
    //             for (auto it=m_features.begin(); it!=m_features.end(); it++)
    //             {
    //                 if ((*it)->id() == id)
    //                 {
    //                     m_features.erase(it);
    //                     return;
    //                 }
    //             }

    //             std::cout << "FeatureCollection::remove() Feature with id '(" << id.dataSetId() << ", " << id.featureId() << ")' doesn't exist!";
    //             throw std::exception();
    //         }

    //         const FeaturePtr& getFeature(const Id& id)
    //         {
    //             for (auto& f : m_features)
    //             {
    //                 if (f->id() == id)
    //                 {
    //                     return f;
    //                 }
    //             }

    //             return nullptr;
    //         }

    //         Rectangle bounds() const 
    //         {
    //             auto boundsList = std::vector<Rectangle>();
    //             for (auto f : m_features) 
    //             { 
    //                 if (!f->bounds().isUndefined())
    //                     boundsList.push_back(f->bounds()); 
    //             }
    //             return Rectangle::mergeBounds(boundsList);
    //         }

    //         std::vector<FeaturePtr>& getVector() { return m_features; }

    //         const FeaturePtr& get(size_t i) { return m_features[i]; }

    //         void clear() { m_features.clear(); }
    //         bool empty() const { return m_features.empty(); }
    //         inline size_t size() const { return m_features.size(); }
    //         inline auto begin() { return m_features.begin(); }
    //         inline auto end() { return m_features.end(); }
    //         inline auto begin() const { return m_features.cbegin(); }
    //         inline auto end() const { return m_features.cend(); }
    //         void reserve(size_t size) { m_features.reserve(size); }

    //     private:
    //         std::vector<FeaturePtr> m_features;
    // };
}

#endif /* BLUEMARBLE_FEATURE */
