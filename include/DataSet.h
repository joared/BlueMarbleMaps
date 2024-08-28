#ifndef BLUEMARBLE_DATASET
#define BLUEMARBLE_DATASET

#include "UpdateInterfaces.h"
#include "File.h"

#include <memory>
#include <map>

namespace BlueMarble
{
    class Map; // Forward declaration

    class DataSet
        : public IUpdateHandler
    {
        public:
            DataSet();
            DataSet(const DataSet&) = delete;
            DataSet& operator=(const DataSet&) = delete;
            Id generateId(FeaturePtr feature);
            FeaturePtr createFeature(GeometryPtr geometry);
            const DataSetId& dataSetId() { return m_dataSetId; }
            virtual void init() = 0;
        private:
            DataSetId m_dataSetId;
    };
    //typedef std::shared_ptr<DataSet> DataSetPtr;

    class ImageDataSet : public DataSet
    {
        public:
            ImageDataSet();
            ImageDataSet(const std::string &filePath);
            void init() override final;
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void OnUpdateRequest(Map& map, const Rectangle& updateArea);
            void OnUpdateRequestOld(Map& map, const Rectangle& updateArea);
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            FeaturePtr onGetFeatureRequest(const Id& id) override final { return nullptr; };

            void filePath(const std::string& filePath);
        private:
            void generateOverViews();

            bool                    m_isInitialized;
            std::string             m_filePath;
            Raster                  m_raster;
            Raster                  m_rasterPrev; // used if scale has not changed;
            std::map<int, Raster>   m_overViews;
    };


    // TODO: this converts features into screen coordinates until Crs class has been implemented
    class AbstractFileDataSet : public DataSet
    {
        public:
            AbstractFileDataSet(const std::string& filePath);
            void init() override final { read(m_filePath); }
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final;
            FeaturePtr onGetFeatureRequest(const Id& id) override final;
        protected:
            void generateOverViews();
            virtual void read(const std::string& filePath) = 0;

            bool                    m_isInitialized;
            std::string             m_filePath;
            std::vector<FeaturePtr> m_features;
    };

    class CsvFileDataSet : public AbstractFileDataSet
    {
        public:
            CsvFileDataSet(const std::string& filePath);
        protected:
            void read(const std::string& filePath) override final;
    };

    class ShapeFileDataSet : public AbstractFileDataSet
    {
        public:
            ShapeFileDataSet(const std::string& filePath);
        protected:
            void read(const std::string& filePath) override final;
    };

    // Specification: https://geojson.org/geojson-spec.html
    class GeoJsonFileDataSet : public AbstractFileDataSet
    {
        public:
            GeoJsonFileDataSet(const std::string& filePath);
        protected:
            void read(const std::string& filePath) override final;
            void save(const std::string& filePath) const;
            void handleJsonData(JsonValue* jsonValue);
            void handleFeatureCollection(JsonValue* jsonValue);
            FeaturePtr handleFeature(JsonValue* jsonValue);
            GeometryPtr handleGeometry(JsonValue* jsonValue);
            Attributes handleProperties(JsonValue *jsonValue);
            GeometryPtr handlePointGeometry(JsonValue* coorcinates);
            GeometryPtr handleLineGeometry(JsonValue* coorcinates);
            GeometryPtr handlePolygon(JsonValue* coorcinates);
            GeometryPtr handleMultiPolygon(JsonValue* coorcinates);
            std::vector<Point> extractPoints(JsonValue* pointList);
            Point extractPoint(JsonValue* point);
            double extractCoordinate(JsonValue* coordinate);
    };

}

#endif /* BLUEMARBLE_DATASET */
