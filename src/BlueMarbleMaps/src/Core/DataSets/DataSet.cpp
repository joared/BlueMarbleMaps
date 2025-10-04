#include "BlueMarbleMaps/Core/DataSets/DataSets.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/FeatureAnimation.h"

#include "BlueMarbleMaps/Utility/Utils.h"

#include <cassert>
#include <set>
#include <thread>
#include <functional>

using namespace BlueMarble;

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


void DataSet::initialize(DataSetInitializationType initType)
{
    assert(!m_isInitialized);
    dataSets[m_dataSetId] = shared_from_this();
    
    auto initWork = [this]()
    {
        try
        {
            m_isInitializing = true;
            init();
            m_isInitializing = false;
        }
        catch(const std::exception& e)
        {
            std::cerr << "Data set initialization failed: " << e.what() << '\n';
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

/* Returns the stored start timestamp for animated visualization if the 
id exists, otherwise -1;*/
int64_t BlueMarble::DataSet::getVisualizationTimeStampForFeature(const Id& id)
{
    auto it = m_idToVisualizationTimeStamp.find(id);
    if (it != m_idToVisualizationTimeStamp.end())
        return it->second;
    
    return -1;
}

std::map<DataSetId, DataSetPtr> DataSet::dataSets;

const std::map<DataSetId, DataSetPtr> &BlueMarble::DataSet::getDataSets()
{
    return dataSets;
}

DataSetPtr DataSet::getDataSetById(const DataSetId &dataSetId)
{
    if (dataSets.find(dataSetId) == dataSets.end())
        return nullptr;
    
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

