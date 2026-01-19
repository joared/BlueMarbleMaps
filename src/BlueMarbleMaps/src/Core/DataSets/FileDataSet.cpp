#include "BlueMarbleMaps/Core/DataSets/FileDataSet.h"
#include "BlueMarbleMaps/Core/Index/QuadTreeIndex.h"
#include "BlueMarbleMaps/Core/Index/DummyIndex.h"
#include "BlueMarbleMaps/Core/Index/FileDatabase.h"
#include "BlueMarbleMaps/Core/Index/MemoryDatabase.h"
#include "BlueMarbleMaps/Core/Index/FIFOCache.h"


using namespace BlueMarble;

AbstractFileDataSet::AbstractFileDataSet(const std::string& filePath, const std::string& indexPath)
    : DataSet() 
    , m_filePath(filePath)
    , m_indexPath(indexPath)
    , m_verifyIndex(false)
    , m_featureStore()
    , m_progress(0)
{
    auto db = std::make_unique<FileDatabase>();
    auto index = std::make_unique<QuadTreeIndex>(Rectangle(-180, -90, 180, 90), 0.05);
    auto cache = std::make_shared<FIFOCache>();
    //cache = nullptr; // Testing without cache
    m_featureStore = std::make_unique<FeatureStore>(dataSetId(), std::move(db), std::move(index), cache);
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
        FeatureCollectionPtr readFeatures = read(m_filePath);
        auto elapsedRead = getTimeStampMs() - startRead;
        BMM_DEBUG() << "Reading took " << elapsedRead << " ms\n";

        for (const auto& f : *readFeatures)
        {
            f->id(generateId());
        }

        BMM_DEBUG() << "Building feature store...\n";
        m_featureStore->build(readFeatures, indexPath);

        m_featureStore->load(indexPath);
    }
    else
    {
        BMM_DEBUG() << "Feature store loaded successfuly!\n";
    }
    
    if (m_verifyIndex)
    {
        BMM_DEBUG() << "Verifying index...\n";
        if (m_featureStore->verifyIndex())
        {
            BMM_DEBUG() << "Verify feature store OK\n";
        }
        else
        {
            BMM_DEBUG() << "Verify feature store Failed\n";
        }
    }

    std::cout << "AbstractFileDataSet::init() Data loaded!\n";
}

double AbstractFileDataSet::progress()
{
    return (isInitialized()) ? 1.0 : (double)m_progress;
}

void AbstractFileDataSet::indexPath(const std::string& indexPath)
{ 
    if (isInitialized())
    {
        throw std::runtime_error("AbstractFileDataSet::indexPath() Data set is already initialized. Index path is only allowed to be modified before initialization.");
    }
    m_indexPath = indexPath;
}

const std::string& AbstractFileDataSet::indexPath()
{
    return m_indexPath;
}


IdCollectionPtr AbstractFileDataSet::onGetFeatureIds(const FeatureQuery& featureQuery)
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

FeatureEnumeratorPtr AbstractFileDataSet::onGetFeatures(const FeatureQuery &featureQuery)
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

FeatureCollectionPtr AbstractFileDataSet::onGetFeatures(const IdCollectionPtr& ids)
{
    auto featureIds = std::make_shared<FeatureIdCollection>();
    featureIds->reserve(ids->size());
    for (const auto& id : *ids)
    {
        if (dataSetId() == id.dataSetId())
            featureIds->add(id.featureId());
    }

    return m_featureStore->getFeatures(featureIds);
}

FeaturePtr AbstractFileDataSet::onGetFeature(const Id &id)
{
    assert(dataSetId() == id.dataSetId());

    return m_featureStore->getFeature(id.featureId());
}

void AbstractFileDataSet::flushCache()
{
    m_featureStore->flushCache();
}