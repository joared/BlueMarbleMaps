#ifndef BLUEMARBLE_FEATURE
#define BLUEMARBLE_FEATURE

#include "Core.h"
#include "Geometry.h"
#include "Identity.h"
#include "Attributes.h"

#include <memory>

namespace BlueMarble
{

    class Feature
    {
        public:
            Feature(const Id& id, GeometryPtr geometry);
            Id id() const;
            Id& id() { return m_id; }; // TODO: remove, temporary fix for setting id
            Rectangle bounds() const;
            Point center() const;
            bool isInside(const Rectangle& bounds) const;
            GeometryType geometryType() const;
            GeometryPtr& geometry();
            PointGeometryPtr geometryAsPoint();
            LineGeometryPtr geometryAsLine();
            PolygonGeometryPtr geometryAsPolygon();
            MultiPolygonGeometryPtr geometryAsMultiPolygon();
            RasterGeometryPtr geometryAsRaster();
            Attributes& attributes();

        private:
            Id          m_id;
            GeometryPtr m_geometry;
            Attributes  m_attributes;
    };
    typedef std::shared_ptr<Feature> FeaturePtr;

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

            inline size_t size() const { return m_features.size(); }
            inline auto begin() const { return m_features.begin(); }
            inline auto end() const { return m_features.end(); }

        private:
            std::vector<FeaturePtr> m_features;
    };
}

#endif /* BLUEMARBLE_FEATURE */
