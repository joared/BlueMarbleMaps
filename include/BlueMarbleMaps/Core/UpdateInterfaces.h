#ifndef UPDATEINTERFACES
#define UPDATEINTERFACES

#include "Feature.h"

namespace BlueMarble
{
    class Map;              // Forward declaration.
    class FeatureHandler;  // Forward declaration.

    // New stuff
    class FeatureQuery
    {
        public:
            enum class RasterGeometryMode
            {
                Original,
                Clipped
            };
            FeatureQuery()
            {

            }
            const Rectangle& area() const { return m_area; }
            void area(const Rectangle& area) { m_area = area; }
            double scale() const { return m_scale; }
            void scale(double scale) { m_scale = scale; }
            RasterGeometryMode rasterGeometryMode() const { return m_rasterMode; }
            void rasterGeometryMode(RasterGeometryMode mode) { m_rasterMode = mode; }
            // Units per pixel in the crs
            double resolution() const { return m_resolution; }
            void resolution(double res) { m_resolution = res; }
            Attributes* updateAttributes() const { return m_updateAttributes; }
            void updateAttributes(Attributes* attr) { m_updateAttributes = attr; }
            bool quickUpdate() const { return m_quickUpdate; }
            void quickUpdate(bool quickpdate) { m_quickUpdate = quickpdate; }
            void ids(const IdCollectionPtr& ids) { m_ids = ids; }
            IdCollectionPtr ids() const { return m_ids; }

        private:
            Rectangle           m_area = Rectangle::infinite();
            IdCollectionPtr     m_ids = std::make_shared<IdCollection>();
            double              m_scale = 1.0;
            Attributes*         m_updateAttributes = nullptr;
            bool                m_quickUpdate = false;
            RasterGeometryMode  m_rasterMode = RasterGeometryMode::Original;
            double              m_resolution=-1.0;
    };

    class FeatureEnumerator;
    typedef std::shared_ptr<FeatureEnumerator> FeatureEnumeratorPtr;
    class FeatureEnumerator
    {
        public:
            FeatureEnumerator(bool isComplete=true);
            void addEnumerator(const FeatureEnumeratorPtr& enumerator) { m_subEnumerators.push_back(enumerator); }
            const std::vector<FeatureEnumeratorPtr>& subEnumerators() const { return m_subEnumerators; }
            const FeaturePtr& current() const;
            bool moveNext();
            void reset();
            void add(const FeaturePtr& feature);
            void setFeatures(const FeatureCollectionPtr& features);
            FeatureCollectionPtr features() const { return m_features; }
            int size();
            bool isComplete() const;
        private:
            bool m_isComplete;
            int m_iteratorIndex;
            int m_iterationIndex;
            FeatureCollectionPtr m_features;
            std::vector<FeatureEnumeratorPtr> m_subEnumerators;
    };
}

#endif /* UPDATEINTERFACES */
