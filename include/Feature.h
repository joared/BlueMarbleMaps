#ifndef BLUEMARBLE_FEATURE
#define BLUEMARBLE_FEATURE

#include "Core.h"
#include "Geometry.h"
#include "Identity.h"
#include "Attributes.h"

#include <memory>

namespace BlueMarble
{

    class Feature; // Forward declaration
    typedef std::shared_ptr<Feature> FeaturePtr;
    class Feature
    {
        public:
            Feature(const Id& id, GeometryPtr geometry);
            Feature(const Id& id, GeometryPtr geometry, const Attributes& attributes);
            FeaturePtr clone();
            Id id() const;
            void id(const Id& id) { m_id = id; }; // TODO: remove, temporary fix for setting id
            void move(const Point& delta);
            void moveTo(const Point& point);
            Rectangle bounds() const;
            Point center() const;
            bool isInside(const Rectangle& bounds) const; // TODO: change name to overlap?
            bool isStrictlyInside(const Rectangle& bounds) const;
            GeometryType geometryType() const;
            GeometryPtr& geometry();
            PointGeometryPtr geometryAsPoint();
            LineGeometryPtr geometryAsLine();
            PolygonGeometryPtr geometryAsPolygon();
            MultiPolygonGeometryPtr geometryAsMultiPolygon();
            RasterGeometryPtr geometryAsRaster();
            Attributes& attributes();
            std::string prettyString();
        private:
            Feature(const Feature&) = default; // Make copy constructor private. Call clone() to copy.
            Id          m_id;
            GeometryPtr m_geometry;
            Attributes  m_attributes;
    };


    class FeatureCollection
    {
        public:
            FeatureCollection()
                : m_features()
            {}
            void add(FeaturePtr feature)
            {
                m_features.push_back(feature);
            }

            void remove(const Id& id)
            {
                for (auto it=m_features.begin(); it!=m_features.end(); it++)
                {
                    if ((*it)->id() == id)
                    {
                        m_features.erase(it);
                        return;
                    }
                }

                std::cout << "FeatureCollection::remove() Feature with id '(" << id.dataSetId() << ", " << id.featureId() << ")' doesn't exist!";
                throw std::exception();
            }

            Rectangle bounds() const 
            {
                auto boundsList = std::vector<Rectangle>();
                for (auto f : m_features) 
                { 
                    if (!f->bounds().isUndefined())
                        boundsList.push_back(f->bounds()); 
                }
                return Rectangle::mergeBounds(boundsList);
            }

            std::vector<FeaturePtr>& getVector() { return m_features; }

            bool empty() const { return m_features.empty(); }
            inline size_t size() const { return m_features.size(); }
            inline auto begin() const { return m_features.begin(); }
            inline auto end() const { return m_features.end(); }

        private:
            std::vector<FeaturePtr> m_features;
    };
}

#endif /* BLUEMARBLE_FEATURE */
