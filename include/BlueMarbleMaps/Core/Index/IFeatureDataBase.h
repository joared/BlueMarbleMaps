#ifndef BLUEMARBLE_IFEATUREDATABASE
#define BLUEMARBLE_IFEATUREDATABASE

#include "BlueMarbleMaps/Core/Feature.h"

namespace BlueMarble
{
    // Interface for a feature data base.
    class IFeatureDataBase
    {
        public:
            virtual ~IFeatureDataBase() = default;

            virtual void addFeature(const FeaturePtr& feature) = 0;
            virtual FeaturePtr getFeature(const FeatureId& id) const = 0;
            virtual FeatureCollectionPtr getFeatures(const FeatureIdCollectionPtr& ids) const = 0;
            virtual void getFeatures(const FeatureIdCollectionPtr& ids, FeatureCollectionPtr& featuresOut) const = 0;
            virtual FeatureCollectionPtr getAllFeatures() const = 0;
            virtual void removeFeature(const FeatureId& id) = 0;
            virtual size_t size() const = 0;

            virtual void save(const std::string& path) const = 0;
            virtual bool load(const std::string& path) = 0;
    };
}

#endif /* BLUEMARBLE_IFEATUREDATABASE */
