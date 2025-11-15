#ifndef IMAGEDATASET
#define IMAGEDATASET

#include "DataSet.h"

namespace BlueMarble
{
    class ImageDataSet : public DataSet
    {
        public:
            ImageDataSet();
            ImageDataSet(const std::string &filePath);
            // void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            // void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            // FeaturePtr onGetFeatureRequest(const Id& id) override final { return nullptr; };

            virtual IdCollectionPtr getFeatureIds(const FeatureQuery& featureQuery) override final;
            virtual FeatureEnumeratorPtr getFeatures(const FeatureQuery& featureQuery) override final;
            virtual FeaturePtr getFeature(const Id& id) override final;

            void filePath(const std::string& filePath);
        private:
            void init() override final;
            void generateOverViews();

            std::string                        m_filePath;
            RasterGeometryPtr                  m_rasterGeometry;
            FeaturePtr                         m_rasterFeature;
            std::map<int, RasterGeometryPtr>   m_overViews;
    };
}

#endif /* IMAGEDATASET */
