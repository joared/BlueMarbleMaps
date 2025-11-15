#include "BlueMarbleMaps/Core/DataSets/FileDataSet.h"
#include "BlueMarbleMaps/Core/Index/QuadTreeIndex.h"
#include "BlueMarbleMaps/Core/Index/FileDatabase.h"
#include "BlueMarbleMaps/Core/Index/MemoryDatabase.h"
#include "BlueMarbleMaps/Core/Index/FIFOCache.h"


using namespace BlueMarble;

AbstractFileDataSet::AbstractFileDataSet(const std::string& filePath, const std::string& indexPath)
    : DataSet() 
    , m_filePath(filePath)
    , m_indexPath(indexPath)
    , m_featureStore()
    , m_progress(0)
{
    //auto db = std::make_unique<MemoryDatabase>();
    auto db = std::make_unique<FileDatabase>();
    auto index = std::make_unique<QuadTreeIndex>(Rectangle(-180, -90, 180, 90), 0.05);
    auto cache = std::make_shared<FIFOCache>();
    m_featureStore = std::make_unique<FeatureStore>(dataSetId(), std::move(db), std::move(index), cache);
}

IdCollectionPtr BlueMarble::AbstractFileDataSet::getFeatureIds(const FeatureQuery& featureQuery)
{
    auto featureIds = m_featureStore->queryIds(featureQuery.area());
    auto ids = std::make_shared<IdCollection>();
    ids->reserve(featureIds->size());
    for (const auto& fid : *featureIds)
    {
        ids->emplace(Id(dataSetId(), fid));
    }

    return ids;
}

FeatureEnumeratorPtr AbstractFileDataSet::getFeatures(const FeatureQuery &featureQuery)
{
    auto enumerator = std::make_shared<FeatureEnumerator>();

    if (!isInitialized()) // TODO: move to base class
    {
        return enumerator;
    }

    FeatureIdCollectionPtr featureIds = nullptr;
    if (!featureQuery.ids()->empty())
    {
        featureIds = std::make_shared<FeatureIdCollection>();
        featureIds->reserve(featureQuery.ids()->size());
        for (const auto& id : *featureQuery.ids())
        {
            assert(id.dataSetId() == dataSetId());
            featureIds->add(id.featureId());
        }
    }
     
    auto features = m_featureStore->query(featureQuery.area(), featureIds);

    enumerator->setFeatures(features);

    return enumerator;
}

void AbstractFileDataSet::init()
{
    if (m_indexPath.empty())
    {
        // We throw an error here such that we don't get index files on random places.
        // Probably change this in the future
        throw std::runtime_error("AbstractFileDataSet::init() index path not set!");
    }

    size_t pos = m_filePath.find_last_of("/\\");
    std::string filename = (pos == std::string::npos) ? m_filePath : m_filePath.substr(pos + 1);
    std::string indexPath = m_indexPath + "/" + filename;

    BMM_DEBUG() << "Loading feature store...\n";
    auto start = getTimeStampMs();
    bool loadOk = m_featureStore->load(indexPath);
    auto elapsed = getTimeStampMs() - start;
    if (loadOk)
        BMM_DEBUG() << "Success! Loading took " << elapsed << " ms\n";
    else
        BMM_DEBUG() << "Feature store not loaded\n";
    
    if (!loadOk)
    {
        BMM_DEBUG() << "Reading features for build...\n";
        auto startRead = getTimeStampMs();
        read(m_filePath); // TODO: change to readFeatures returning FeatureCollectionPtr
        auto elapsedRead = getTimeStampMs() - startRead;
        BMM_DEBUG() << "Reading took " << elapsedRead << " ms\n";

        auto featureColl = std::make_shared<FeatureCollection>();
        for (const auto& f : m_features)
        {
            f->id(generateId());
            featureColl->add(f);
        }

        BMM_DEBUG() << "Building feature store...\n";
        m_featureStore->buildIndex(featureColl, indexPath);

        m_featureStore->load(indexPath);

        if (m_featureStore->verifyIndex())
        {
            BMM_DEBUG() << "Verify feature store OK\n";
        }
        else
        {
            BMM_DEBUG() << "Verify feature store Failed\n";
        }
    }
    else
    {
        BMM_DEBUG() << "Feature store loaded successfuly!\n";
    }
    
    std::cout << "AbstractFileDataSet::init() Data loaded!\n";
}

double AbstractFileDataSet::progress()
{
    return (isInitialized()) ? 1.0 : (double)m_progress;
}

void BlueMarble::AbstractFileDataSet::indexPath(const std::string& indexPath)
{ 
    if (isInitialized())
    {
        throw std::runtime_error("AbstractFileDataSet::indexPath() Data set is already initialized. Index path is only allowed to be modified before initialization.");
    }
    m_indexPath = indexPath;
}

const std::string& BlueMarble::AbstractFileDataSet::indexPath()
{
    return m_indexPath;
}

FeaturePtr AbstractFileDataSet::getFeature(const Id &id)
{
    std::cout << "AbstractFileDataSet::onGetFeatureRequest\n";
    for (auto f : m_features)
    {
        if (f->id() == id)
        {
            std::cout << "Found a match!\n";
            return f;
        }
    }

    return nullptr;
}

void AbstractFileDataSet::flushCache()
{
}