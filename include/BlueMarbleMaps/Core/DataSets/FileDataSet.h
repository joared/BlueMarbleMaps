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

            virtual FeatureEnumeratorPtr getFeatures(const FeatureQuery& featureQuery) override final;
            virtual FeaturePtr getFeature(const Id& id) override final;

        protected:
            void init() override final;
            virtual void read(const std::string& filePath) = 0; // TODO: change to readFeatures returning FeatureCollectionPtr

            std::string                    m_filePath;
            std::string                    m_indexPath;
            std::vector<FeaturePtr>        m_features;
            std::unique_ptr<FeatureStore>  m_featureStore;
            std::atomic<double>            m_progress;
    };
}

#endif /* BLUEMARBLE_FILEDATASET */
