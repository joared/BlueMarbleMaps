#ifndef BLUEMARBLE_FEATURESTORE
#define BLUEMARBLE_FEATURESTORE

#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/Index/ISpatialIndex.h"
#include "BlueMarbleMaps/Core/Index/IFeatureDataBase.h"
#include "BlueMarbleMaps/Core/Index/IFeatureCache.h"

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
        FeatureCollectionPtr query(const Rectangle& area);

        bool load(const std::string& indexPath);
        void save(const std::string& indexPath);

        bool verifyIndex() const;
    private:
        Id toValidId(const FeatureId& featureId);

        DataSetId                           m_dataSetId;
        std::unique_ptr<IFeatureDataBase>   m_dataBase;
        std::unique_ptr<ISpatialIndex>      m_index;
        IFeatureCachePtr                    m_cache;
    };
}

#endif /* BLUEMARBLE_FEATURESTORE */
