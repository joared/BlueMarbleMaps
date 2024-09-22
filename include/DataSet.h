#ifndef BLUEMARBLE_DATASET
#define BLUEMARBLE_DATASET

#include "UpdateInterfaces.h"
#include "File.h"
#include "Algorithm.h"
#include "EngineObject.h"

#include <atomic>
#include <memory>
#include <map>

namespace BlueMarble
{
    class Map; // Forward declaration

    enum class DataSetInitializationType
    {
        RightHereRightNow,
        BackgroundThread
    };

    class DataSet
        : public EngineObject
        , public IUpdateHandler
    {
        public:
            DataSet();
            DataSet(const DataSet&) = delete;
            DataSet& operator=(const DataSet&) = delete;
            Id generateId();
            FeaturePtr createFeature(GeometryPtr geometry);
            const DataSetId& dataSetId() { return m_dataSetId; }
            virtual void init(DataSetInitializationType initType = DataSetInitializationType::BackgroundThread) = 0;
        private:
            DataSetId m_dataSetId;
    };
    //typedef std::shared_ptr<DataSet> DataSetPtr;


    class ImageDataSet : public DataSet
    {
        public:
            ImageDataSet();
            ImageDataSet(const std::string &filePath);
            void init(DataSetInitializationType initType = DataSetInitializationType::BackgroundThread) override final;
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void OnUpdateRequest(Map& map, const Rectangle& updateArea);
            void OnUpdateRequestOld(Map& map, const Rectangle& updateArea);
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            FeaturePtr onGetFeatureRequest(const Id& id) override final { return nullptr; };

            void filePath(const std::string& filePath);
        private:
            void generateOverViews();

            std::atomic<bool>       m_isInitialized;
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
            void init(DataSetInitializationType initType = DataSetInitializationType::BackgroundThread) override final;
            double progress();
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final;
            FeaturePtr onGetFeatureRequest(const Id& id) override final;
        protected:
            virtual void read(const std::string& filePath) = 0;

            std::string             m_filePath;
            std::vector<FeaturePtr> m_features;
            Algorithm::QuadTree     m_featureTree;
            std::atomic<bool>       m_isInitialized;
            std::atomic<double>     m_progress;
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

    class MemoryDataSet : public DataSet
    {
        public:
            MemoryDataSet();
            void init(DataSetInitializationType initType = DataSetInitializationType::RightHereRightNow);
            void addFeature(FeaturePtr feature);
            void removeFeature(const Id& id);
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            FeaturePtr onGetFeatureRequest(const Id& id) override final { return nullptr; };
        private:
            FeatureCollection m_features;
    };

}

#endif /* BLUEMARBLE_DATASET */
