#ifndef BLUEMARBLE_MEMORYDATABASE
#define BLUEMARBLE_MEMORYDATABASE

#include "IFeatureDataBase.h"
#include "FIFOCache.h"

namespace BlueMarble
{
    // A database that simply wraps a FeatureCahce instance so that all features are stored in RAM.
    class MemoryDatabase : public IFeatureDataBase
    {
    public:
        MemoryDatabase();
        
        virtual FeaturePtr getFeature(const FeatureId& id) override final;
        virtual FeatureCollectionPtr getFeatures(const FeatureIdCollectionPtr& ids) override final;
        virtual void getFeatures(const FeatureIdCollectionPtr& ids, FeatureCollectionPtr& featuresOut) override final;
        virtual FeatureCollectionPtr getAllFeatures() override final;
        virtual void removeFeature(const FeatureId& id) override final;
        virtual size_t size() const override final;

        virtual void save(const std::string& path) const override final;
        virtual bool load(const std::string& path) override final;
        virtual bool build(const FeatureCollectionPtr& features, const std::string& path) override final;
    private:
        FIFOCache m_cache;
    };
}

#endif /* BLUEMARBLE_MEMORYDATABASE */
