#ifndef BLUEMARBLE_FEATURECACHE
#define BLUEMARBLE_FEATURECACHE

#include "BlueMarbleMaps/Core/Feature.h"

#include <map>

namespace BlueMarble
{
    class IFeatureCache
    {
    public:
        virtual ~IFeatureCache() = default;
        virtual void insert(const Id& id, const FeaturePtr& feature) = 0;
        virtual void remove(const Id& id) = 0;
        virtual bool contains(const Id& id) const = 0;
        virtual const FeaturePtr& getFeature(const Id& feature) const = 0;
        virtual FeatureCollectionPtr getAllFeatures() const = 0;
        virtual size_t size() const = 0;
        virtual void clear() = 0;
    };
    typedef std::shared_ptr<IFeatureCache> IFeatureCachePtr;
}

#endif /* BLUEMARBLE_FEATURECACHE */
