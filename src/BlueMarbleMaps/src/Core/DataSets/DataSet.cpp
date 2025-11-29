#include "BlueMarbleMaps/Core/DataSets/DataSets.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/FeatureAnimation.h"

#include "BlueMarbleMaps/Utility/Utils.h"

#include <cassert>
#include <set>
#include <thread>
#include <functional>

using namespace BlueMarble;

std::map<DataSetId, DataSetPtr> DataSet::dataSets;
DataSet::GlobalDataSetEvents DataSet::globalEvents;

const std::map<DataSetId, DataSetPtr> &BlueMarble::DataSet::getDataSets()
{
    return dataSets;
}

DataSetPtr DataSet::getDataSetById(const DataSetId &dataSetId)
{
    auto it = dataSets.find(dataSetId);
    if (it == dataSets.end())
    {
        std::string availableDataSetIds;
        for (const auto& el : dataSets)
        {
            availableDataSetIds += std::to_string(el.first) + "\n";
        }
        throw std::runtime_error("DataSet::getDataSetById() Available data sets:\n" + availableDataSetIds + "\nCould not find data set with id: " + std::to_string(dataSetId));
    }
    
    return dataSets[dataSetId];
}

DataSet::DataSet()
    : m_dataSetId(DataSetId(this))
    , m_featureIdCounter(0)
    , m_crs(Crs::wgs84LngLat()) // TODO: should be nullptr?
    , m_isInitialized(false)
    , m_isInitializing(false)
    , m_idToVisualizationTimeStamp()
{
}

DataSet::~DataSet()
{
    dataSets.erase(m_dataSetId);
}

void DataSet::initialize(DataSetInitializationType initType)
{
    assert(!m_isInitialized);
    assert(!m_isInitializing);

    dataSets[m_dataSetId] = shared_from_this();
    
    auto initWork = [this]()
    {
        bool surpressErrors = false;
        try
        {
            m_isInitializing = true;
            globalEvents.onInitializing.notify(shared_from_this());
            init();
            globalEvents.onInitialized.notify(shared_from_this());
            m_isInitializing = false;
        }
        catch(const std::exception& e)
        {
            std::cerr << "Data set initialization failed: " << e.what() << '\n';
            if (!surpressErrors) throw e;
            m_isInitialized = false;
            return;
        }
        
        m_isInitialized = true;
    };

    if (initType == DataSetInitializationType::RightHereRightNow)
    {
        BMM_DEBUG() << "Direct initializaton of data set " << dataSetId() << "\n";
        initWork();
    }
    else
    {
        BMM_DEBUG() << "Started init thread for data set " << dataSetId() << "\n";
        std::thread readThread(initWork);
        readThread.detach();
    }
}

Id BlueMarble::DataSet::generateId()
{
    ++m_featureIdCounter;
    return Id(m_dataSetId, m_featureIdCounter);
}

FeaturePtr DataSet::createFeature(GeometryPtr geometry)
{
    assert(m_crs);

    auto feature = std::make_shared<Feature>
    (
        generateId(),
        m_crs,
        geometry
    );

    return feature;
}

void DataSet::restartVisualizationAnimation(FeaturePtr feature, int64_t timeStamp)
{
    assert(feature->id().dataSetId() == dataSetId());
    
    if (timeStamp == -1)
        timeStamp = getTimeStampMs();

    //feature->attributes().set(FeatureAttributeKeys::StartAnimationTimeMs, timeStamp);
    m_idToVisualizationTimeStamp[feature->id()] = timeStamp;
}

bool DataSet::ensureInitialized()
{
    if (!m_isInitialized && !m_isInitializing)
    {
        initialize();
        return false;
    }
    else if (m_isInitializing)
    {
        return false;
    }                  
    
    return true;
} 

IdCollectionPtr DataSet::getFeatureIds(const FeatureQuery& featureQuery)
{
    if (!ensureInitialized()) return std::make_shared<IdCollection>();
    return queryFeatureIds(featureQuery);
}

FeatureEnumeratorPtr DataSet::getFeatures(const FeatureQuery& featureQuery)
{
    if (!ensureInitialized()) return std::make_shared<FeatureEnumerator>();
    return queryFeatures(featureQuery);
}

FeatureCollectionPtr DataSet::getFeatures(const IdCollectionPtr& ids)
{
    if (!ensureInitialized()) return std::make_shared<FeatureCollection>();
    return queryFeatures(ids);
}

FeaturePtr DataSet::getFeature(const Id& id)
{
    if (!ensureInitialized()) return nullptr;
    return queryFeature(id);
}

FeatureCollectionPtr DataSet::queryFeatures(const IdCollectionPtr& ids)
{
    auto features = std::make_shared<FeatureCollection>();
    for (const auto& id : *ids)
    {
        features->add(queryFeature(id));
    }
    
    return features;
}


/* Returns the stored start timestamp for animated visualization if the
 id exists, otherwise -1;*/
int64_t BlueMarble::DataSet::getVisualizationTimeStampForFeature(const Id& id)
{
    auto it = m_idToVisualizationTimeStamp.find(id);
    if (it != m_idToVisualizationTimeStamp.end())
        return it->second;
    
    return -1;
}