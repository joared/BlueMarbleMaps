#include "BlueMarbleMaps/Core/Index/FeatureStore.h"
#include <fstream>


using namespace BlueMarble;

FeatureStore::FeatureStore(const DataSetId& dataSetId,
                           std::unique_ptr<IFeatureDataBase> dataBase, 
                           std::unique_ptr<ISpatialIndex> index,
                           IFeatureCachePtr cache)
    : m_dataSetId(dataSetId)
    , m_dataBase(std::move(dataBase))
    , m_index(std::move(index))
    , m_cache(cache)
{
    
}

void FeatureStore::addFeature(const FeaturePtr &feature)
{
    throw std::runtime_error("FeatureStore::addFeature() Not implemented.");
}

FeaturePtr FeatureStore::getFeature(const FeatureId &id)
{
    auto f = m_dataBase->getFeature(id);
    if (f)
    {
        f->id(toValidId(id));
    }

    return f;
}

FeatureCollectionPtr FeatureStore::query(const Rectangle& area)
{
    // TODO: maybe store a member collection with preallocated size
    auto features = std::make_shared<FeatureCollection>();
    features->reserve(10000); // TODO: Fix this
    features->clear();

    auto featureIds = m_index->query(area);

    // First try to retrieve the features from the cache
    auto cacheMissingIds = std::make_shared<FeatureIdCollection>();
    if (m_cache)
    {
        for (const auto& featureId : *featureIds)
        {
            auto id = Id(m_dataSetId, featureId);
            if (m_cache->contains(id))
            {
                features->add(m_cache->getFeature(id));
            }
            else
            {
                cacheMissingIds->add(featureId);
            }
        }
    }
    else
    {
        cacheMissingIds->addRange(*featureIds);
    }

    // Get missing features from the database
    if (!cacheMissingIds->empty())
    {
        auto nonCachedFeatures = std::make_shared<FeatureCollection>();
        m_dataBase->getFeatures(cacheMissingIds, nonCachedFeatures);

        // Add the noncached features to the cache
        if (m_cache)
        {
            for (const auto& f : *nonCachedFeatures)
            {
                //break; // Testing without cache to see performance
                f->id(Id(m_dataSetId, f->id().featureId())); // The database has no idea about the dataset id, but we do!
                m_cache->insert(f->id(), f);
            }
        }

        // Add the noncached features to the result
        features->addRange(*nonCachedFeatures);

        BMM_DEBUG() << "FeatureStore::query()\n";
        BMM_DEBUG() << "Queried " << nonCachedFeatures->size() << " new features from database\n";
        BMM_DEBUG() << "Database size: " << m_dataBase->size() << "\n";
        //BMM_DEBUG() << "Index size: " << m_index->size() << "\n";
        //BMM_DEBUG() << "Cache size: " << m_cache->size() << "\n";
        BMM_DEBUG() << "-------------------------\n";
    }

    return features;
}

bool FeatureStore::load(const std::string& indexPath)
{
    // We never rebuild database on load, it has to be done eplicitly since it takes time
    if (!m_dataBase->load(indexPath + ".database"))
        return false;

    bool indexOk = m_index->load(indexPath + ".index");
    
    // We allow to rebuild the index on load
    if (!indexOk)
    {
        auto features = m_dataBase->getAllFeatures();
        m_index->build(features, indexPath + ".index");
    }

    return true;
}


bool FeatureStore::verifyIndex() const
{
    auto featureIds = m_index->queryAll();
    auto features = m_dataBase->getAllFeatures();
    size_t indexSize = featureIds->size();
    size_t dataSize = features->size();

    if (indexSize != dataSize)
    {
        BMM_DEBUG() << "FeatureStore::verifyIndex() size missmatch: " << indexSize << " != " << dataSize << "\n";   
        return false;
    }

    for (const auto f : *features)
    {
        if (!featureIds->contains(f->id().featureId()))
        {
            // A feature id is missing
            BMM_DEBUG() << "FeatureStore::verifyIndex() id missing in index: " << f->id().toString() << "\n";
            return false;
        }
    }

    return true;
}

void FeatureStore::flushCache()
{
    m_cache->clear();
}

void FeatureStore::buildIndex(const FeatureCollectionPtr &features, const std::string &indexPath)
{
    for (const auto& feature : *features)
    {
        // We can only add features associated with one data set
        assert(feature->id().dataSetId() == m_dataSetId);
    }

    m_dataBase->build(features, indexPath + ".database");
    m_index->build(features, indexPath + ".index");
}


Id FeatureStore::toValidId(const FeatureId& featureId)
{
    return Id(m_dataSetId, featureId);
}
