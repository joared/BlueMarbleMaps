#ifndef BLUEMARBLE_DATASET
#define BLUEMARBLE_DATASET

#include "BlueMarbleMaps/Core/UpdateInterfaces.h"
#include "BlueMarbleMaps/Utility/Algorithm.h"
#include "BlueMarbleMaps/Core/EngineObject.h"
#include "BlueMarbleMaps/Core/FeatureAnimation.h"
#include "BlueMarbleMaps/CoordinateSystem/Crs.h"
#include "BlueMarbleMaps/System/File.h"

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
            static const std::map<DataSetId, DataSetPtr>& getDataSets(); 
            static DataSetPtr getDataSetById(const DataSetId& dataSetId);
            DataSet();
            virtual ~DataSet();
            DataSet(const DataSet&) = delete;
            DataSet& operator=(const DataSet&) = delete;
            Id generateId();
            FeaturePtr createFeature(GeometryPtr geometry);
            const DataSetId& dataSetId() { return m_dataSetId; }
            const CrsPtr& getCrs() { return m_crs; }
            void setCrs(const CrsPtr& crs) { m_crs = crs; }
            bool isInitialized() { return m_isInitialized; }
            bool isInitializing() { return m_isInitializing; }
            void initialize(DataSetInitializationType initType = DataSetInitializationType::BackgroundThread);
            int64_t getVisualizationTimeStampForFeature(const Id& id);
            void restartVisualizationAnimation(FeaturePtr feature, int64_t timeStamp = -1);
        protected:
            virtual void init() = 0;
        private:
            DataSetId        m_dataSetId;
            CrsPtr           m_crs;
            std::atomic_bool m_isInitialized;
            std::atomic_bool m_isInitializing;
            std::map<Id, int64_t> m_idToVisualizationTimeStamp;
    };


    class ImageDataSet : public DataSet
    {
        public:
            ImageDataSet();
            ImageDataSet(const std::string &filePath);
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override final;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override final {};
            FeaturePtr onGetFeatureRequest(const Id& id) override final { return nullptr; };

            void filePath(const std::string& filePath);
        private:
            void init() override final;
            void generateOverViews();

            std::string                        m_filePath;
            RasterGeometryPtr                  m_rasterGeometry;
            std::map<int, RasterGeometryPtr>   m_overViews;
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

    class IFeatureEventListener
    {
        public:
            virtual void onFeatureCreated(const FeaturePtr& feature) = 0;
            virtual void onFeatureUpdated(const FeaturePtr& feature) = 0;
            virtual void onFeatureDeleted(const Id& id) = 0;
    };

    class FeatureEventPublisher
    {
        public:
            FeatureEventPublisher(bool eventsEnabled);
            void addFeatureEventListener(IFeatureEventListener* listener, const Id& id);
            void removeFeatureEventListener(IFeatureEventListener* listener, const Id& id);
            bool featureEventsEnabled();
            void featureEventsEnabled(bool enabled);
        protected:
            void sendOnFeatureCreated(const FeaturePtr& feature) const;
            void sendOnFeatureUpdated(const FeaturePtr& feature) const;
            void sendOnFeatureDeleted(const Id& id) const;
        private:
            std::map<Id, std::vector<IFeatureEventListener*>> m_listeners;
            bool m_eventsEnabled;
    };

    class MemoryDataSet 
        : public DataSet
        , public FeatureEventPublisher
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
            void triggerFeatureUpdated(const FeaturePtr& id);
        protected:
            void init() override final;
        private:
            FeatureCollection m_features;
            std::map<Id, FeatureAnimationPtr> m_idToFeatureAnimation;
    };
    typedef std::shared_ptr<MemoryDataSet> MemoryDataSetPtr;

}

#endif /* BLUEMARBLE_DATASET */
