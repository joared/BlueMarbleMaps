#ifndef BLUEMARBLE_FILEDATASET
#define BLUEMARBLE_FILEDATASET

#include "DataSet.h"
#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Utility/Algorithm.h"
#include "BlueMarbleMaps/Core/UpdateInterfaces.h"

namespace BlueMarble
{
    // TODO: this converts features into screen coordinates until Crs class has been implemented
    class AbstractFileDataSet : public DataSet
    {
        public:
            AbstractFileDataSet(const std::string& filePath);
            double progress();
            // void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            // void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final;
            // FeaturePtr onGetFeatureRequest(const Id& id) override final;

            virtual FeatureEnumeratorPtr getFeatures(const FeatureQuery& featureQuery) override final;
            virtual FeaturePtr getFeature(const Id& id) override final;

        protected:
            void init() override final;
            virtual void read(const std::string& filePath) = 0;

            std::string             m_filePath;
            std::vector<FeaturePtr> m_features;
            Algorithm::QuadTree     m_featureTree;
            std::atomic<double>     m_progress;
    };
}

#endif /* BLUEMARBLE_FILEDATASET */
