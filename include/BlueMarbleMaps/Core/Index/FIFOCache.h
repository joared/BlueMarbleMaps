#ifndef BLUEMARBLE_FIFOCACHE
#define BLUEMARBLE_FIFOCACHE

#include "IFeatureCache.h"

namespace BlueMarble
{
    class FIFOCache : public IFeatureCache
    {
    public:
        FIFOCache();
        virtual void insert(const Id& id, const FeaturePtr& feature) override final;
        virtual void remove(const Id& id) override final;
        virtual bool contains(const Id& id) const override final;
        virtual const FeaturePtr& getFeature(const Id& feature) const override final;
        virtual FeatureCollectionPtr getAllFeatures() const override final;
        virtual size_t size() const override final;
        virtual void clear() override final;
    private:
        std::unordered_map<Id, FeaturePtr, IdHash> m_cache;
    };

    typedef std::shared_ptr<FIFOCache> FIFOCachePtr;
}

#endif /* BLUEMARBLE_FIFOCACHE */
