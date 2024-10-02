#ifndef BLUEMARBLE_DATASET
#define BLUEMARBLE_DATASET

#include "UpdateInterfaces.h"
#include "File.h"
#include "Algorithm.h"
#include "EngineObject.h"
#include "FeatureAnimation.h"

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

    class DataSet; // Forward declaration
    typedef std::shared_ptr<DataSet> DataSetPtr;
    class DataSet 
        : public std::enable_shared_from_this<DataSet>
        , public EngineObject
        , public IUpdateHandler
    {
        private:
            static std::map<DataSetId, DataSetPtr> dataSets;
        public:
            static DataSetPtr getDataSetById(const DataSetId& dataSetId);
            DataSet();
            virtual ~DataSet();
            DataSet(const DataSet&) = delete;
            DataSet& operator=(const DataSet&) = delete;
            Id generateId();
            FeaturePtr createFeature(GeometryPtr geometry);
            const DataSetId& dataSetId() { return m_dataSetId; }
            bool isInitialized() { return m_isInitialized; }
            void initialize(DataSetInitializationType initType = DataSetInitializationType::BackgroundThread);
            int getVisualizationTimeStampForFeature(const Id& id);
            void restartVisualizationAnimation(FeaturePtr feature, int timeStamp = getTimeStampMs());
        protected:
            virtual void init() = 0;
        private:
            DataSetId        m_dataSetId;
            std::atomic_bool m_isInitialized;
            std::map<Id, int> m_idToVisualizationTimeStamp;
    };


    class ImageDataSet : public DataSet
    {
        public:
            ImageDataSet();
            ImageDataSet(const std::string &filePath);
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void OnUpdateRequest(Map& map, const Rectangle& updateArea);
            void OnUpdateRequestOld(Map& map, const Rectangle& updateArea);
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            FeaturePtr onGetFeatureRequest(const Id& id) override final { return nullptr; };

            void filePath(const std::string& filePath);
        private:
            void init() override final;
            void generateOverViews();

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
            double progress();
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final;
            FeaturePtr onGetFeatureRequest(const Id& id) override final;
        protected:
            void init() override final;
            virtual void read(const std::string& filePath) = 0;

            std::string             m_filePath;
            std::vector<FeaturePtr> m_features;
            Algorithm::QuadTree     m_featureTree;
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
            void addFeature(FeaturePtr feature);
            void removeFeature(const Id& id);
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            FeaturePtr onGetFeatureRequest(const Id& id) override final;
            void startFeatureAnimation(FeaturePtr feature);
            void startFeatureAnimation(FeaturePtr feature, const Point& from, const Point& to);
        protected:
            void init() override final;
        private:
            FeatureCollection m_features;
            std::map<Id, FeatureAnimationPtr> m_idToFeatureAnimation;
    };

}

#endif /* BLUEMARBLE_DATASET */
