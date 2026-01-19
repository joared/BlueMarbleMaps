#ifndef BLUEMARBLE_FEATURESTORE
#define BLUEMARBLE_FEATURESTORE

#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/Index/ISpatialIndex.h"
#include "BlueMarbleMaps/Core/Index/IFeatureDataBase.h"
#include "BlueMarbleMaps/Core/Index/IFeatureCache.h"
#include "BlueMarbleMaps/Core/Index/IPersistable.h"

#include <memory>

namespace BlueMarble
{
    class FeatureStore
    {
    public:
        FeatureStore(const DataSetId& dataSetId,
                     std::unique_ptr<IFeatureDataBase> dataBase,
                     std::unique_ptr<ISpatialIndex> index,
                     IFeatureCachePtr cache = nullptr); // The cache is optional

        void addFeature(const FeaturePtr& feature);

        FeaturePtr getFeature(const FeatureId& id);
        FeatureCollectionPtr getFeatures(const FeatureIdCollectionPtr& ids);
        FeatureIdCollectionPtr queryIds(const Rectangle& area);
        FeatureCollectionPtr query(const Rectangle& area, const FeatureIdCollectionPtr& featureIds=nullptr);

        void build(const FeatureCollectionPtr& features, const std::string& indexPath);
        bool load(const std::string& indexPath);
        bool verifyIndex() const;

        void flushCache();
    private:
        static IPersistable::PersistanceContext getIndexPersistanceContext(IPersistable* p, const std::string& indexPath);
        static IPersistable::PersistanceContext getDatabasePersistanceContext(IPersistable* p, const std::string& indexPath);
        Id toValidId(const FeatureId& featureId);
        static FeatureIdCollectionPtr idIntersection(const FeatureIdCollectionPtr& requested, const FeatureIdCollectionPtr& candidates);

        DataSetId                           m_dataSetId;
        std::unique_ptr<IFeatureDataBase>   m_dataBase;
        std::unique_ptr<ISpatialIndex>      m_index;
        IFeatureCachePtr                    m_cache;
    };
}

#endif /* BLUEMARBLE_FEATURESTORE */
