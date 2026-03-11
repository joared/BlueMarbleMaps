#include "BlueMarbleMaps/Core/DataSets/DataSets.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/FeatureAnimation.h"

#include "BlueMarbleMaps/Utility/Utils.h"

#include <cassert>
#include <set>
#include <thread>
#include <functional>

using namespace BlueMarble;

std::mutex                      DataSet::s_dataSetsMutex;
std::map<DataSetId, DataSetPtr> DataSet::s_dataSets;
DataSet::GlobalDataSetEvents    DataSet::s_globalEvents;

DataSetPtr DataSet::getDataSetById(const DataSetId &dataSetId)
{
    std::lock_guard lock(s_dataSetsMutex);

    auto it = s_dataSets.find(dataSetId);
    if (it == s_dataSets.end())
    {
        std::string availableDataSetIds;
        for (const auto& el : s_dataSets)
        {
            availableDataSetIds += std::to_string(el.first) + "\n";
        }
        throw std::runtime_error("DataSet::getDataSetById() Available data sets:\n" + availableDataSetIds + "\nCould not find data set with id: " + std::to_string(dataSetId));
    }
    
    return s_dataSets[dataSetId];
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
    {
        std::lock_guard lock(s_dataSetsMutex);
        s_dataSets.erase(m_dataSetId);
    }
}

void DataSet::initialize(DataSetInitializationType initType)
{
    assert(!m_isInitialized);
    assert(!m_isInitializing);

    {
        std::lock_guard lock(s_dataSetsMutex);
        s_dataSets[m_dataSetId] = shared_from_this();
    }
    
    auto initWork = [this]()
    {
        // If we want this true, we need to properly 
        // add handling for an invalid dataset
        bool surpressErrors = false;
        m_isInitializing = true;
        s_globalEvents.onInitializing.notify(shared_from_this());
        
        try
        {
            init();
            
        }
        catch(const std::exception& e)
        {
            std::cerr << "Data set initialization failed: " << e.what() << '\n';

            m_isInitializing = false;
            s_globalEvents.onInitializationFailed.notify(shared_from_this());

            if (!surpressErrors) throw e;
            
            return;
        }
        m_isInitializing = false;
        m_isInitialized = true;
        s_globalEvents.onInitialized.notify(shared_from_this());
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
    return onGetFeatureIds(featureQuery);
}

FeatureEnumeratorPtr DataSet::getFeatures(const FeatureQuery& featureQuery)
{
    // Return incomplete enumerator if not initialized to avoid blocking the caller with initialization. 
    // Caller can check if the enumerator is complete and decide to query again later or show a loading indicator or something.
    if (!ensureInitialized()) return std::make_shared<FeatureEnumerator>(false);
    return onGetFeatures(featureQuery);
}

FeatureCollectionPtr DataSet::getFeatures(const IdCollectionPtr& ids)
{
    if (!ensureInitialized()) return std::make_shared<FeatureCollection>();
    return onGetFeatures(ids);
}

FeaturePtr DataSet::getFeature(const Id& id)
{
    if (!ensureInitialized()) return nullptr;
    return onGetFeature(id);
}

FeatureCollectionPtr DataSet::onGetFeatures(const IdCollectionPtr& ids)
{
    auto features = std::make_shared<FeatureCollection>();
    for (const auto& id : *ids)
    {
        features->add(onGetFeature(id));
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