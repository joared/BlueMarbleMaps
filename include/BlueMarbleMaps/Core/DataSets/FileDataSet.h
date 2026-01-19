#ifndef BLUEMARBLE_FILEDATASET
#define BLUEMARBLE_FILEDATASET

#include "DataSet.h"
#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/UpdateInterfaces.h"
#include "BlueMarbleMaps/Core/Index/FeatureStore.h"

namespace BlueMarble
{
    // TODO: this converts features into screen coordinates until Crs class has been implemented
    class AbstractFileDataSet : public DataSet
    {
        public:
            AbstractFileDataSet(const std::string& filePath, const std::string& indexPath="");
            double progress();
            void indexPath(const std::string& indexPath);
            const std::string& indexPath();
            void verifyIndex(bool verify) { m_verifyIndex = verify; }

            virtual void flushCache() override final;
        protected:
            virtual IdCollectionPtr onGetFeatureIds(const FeatureQuery& featureQuery) override final;
            virtual FeatureEnumeratorPtr onGetFeatures(const FeatureQuery& featureQuery) override final;
            virtual FeatureCollectionPtr onGetFeatures(const IdCollectionPtr& id) override final;
            virtual FeaturePtr onGetFeature(const Id& id) override final;
            void init() override final;
            virtual FeatureCollectionPtr read(const std::string& filePath) = 0; // TODO: change to readFeatures returning FeatureCollectionPtr

            std::string                    m_filePath;
            std::string                    m_indexPath;
            bool                           m_verifyIndex;
            std::unique_ptr<FeatureStore>  m_featureStore;
            std::atomic<double>            m_progress;
    };
}

#endif /* BLUEMARBLE_FILEDATASET */
