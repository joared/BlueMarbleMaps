#ifndef BLUEMARBLE_DATASET
#define BLUEMARBLE_DATASET

#include "BlueMarbleMaps/Core/UpdateInterfaces.h"
#include "BlueMarbleMaps/Core/ResourceObject.h"
#include "BlueMarbleMaps/CoordinateSystem/Crs.h"
#include "BlueMarbleMaps/System/File.h"
#include "BlueMarbleMaps/System/JsonFile.h"

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
        , public ResourceObject
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
            const CrsPtr& crs() { return m_crs; }
            void crs(const CrsPtr& crs) { m_crs = crs; }
            bool isInitialized() { return m_isInitialized; }
            bool isInitializing() { return m_isInitializing; }
            void initialize(DataSetInitializationType initType = DataSetInitializationType::RightHereRightNow);
            int64_t getVisualizationTimeStampForFeature(const Id& id);
            void restartVisualizationAnimation(FeaturePtr feature, int64_t timeStamp = -1);

            virtual IdCollectionPtr getFeatureIds(const FeatureQuery& featureQuery) = 0;
            virtual FeatureEnumeratorPtr getFeatures(const FeatureQuery& featureQuery) = 0;
            virtual FeaturePtr getFeature(const Id& id) = 0;
            virtual void flushCache() {}; // TODO: make pure virtual?

        protected:
            virtual void init() = 0;
        private:
            DataSetId        m_dataSetId;
            FeatureId        m_featureIdCounter;
            CrsPtr           m_crs;
            std::atomic_bool m_isInitialized;
            std::atomic_bool m_isInitializing;
            std::map<Id, int64_t> m_idToVisualizationTimeStamp;
    };
}

#endif /* BLUEMARBLE_DATASET */
