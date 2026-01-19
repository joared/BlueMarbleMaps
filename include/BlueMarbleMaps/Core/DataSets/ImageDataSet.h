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
            void filePath(const std::string& filePath);
        private:
            void init() override final;
            void generateOverViews();
            virtual IdCollectionPtr onGetFeatureIds(const FeatureQuery& featureQuery) override final;
            virtual FeatureEnumeratorPtr onGetFeatures(const FeatureQuery& featureQuery) override final;
            virtual FeaturePtr onGetFeature(const Id& id) override final;
            

            std::string                        m_filePath;
            RasterGeometryPtr                  m_rasterGeometry;
            FeaturePtr                         m_rasterFeature;
            std::map<int, RasterGeometryPtr>   m_overViews;
    };
}

#endif /* IMAGEDATASET */
