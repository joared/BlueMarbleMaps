#include "BlueMarbleMaps/Core/Index/FeatureStore.h"

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

void FeatureStore::addFeature(const FeaturePtr& feature)
{
    // We can only add features associated with one data set
    assert(feature->id().dataSetId() == m_dataSetId);

    m_dataBase->addFeature(feature);
    m_index->insert(feature->id().featureId(), feature->bounds());
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

FeatureCollectionPtr FeatureStore::query(const Rectangle &area)
{
    static auto features = std::make_shared<FeatureCollection>();
    features->reserve(10000); // TODO: Fix this
    features->clear();

    auto featureIds = m_index->query(area);
    
    BMM_DEBUG() << "FeatureStore::query()\n";
    BMM_DEBUG() << "Database size: " << m_dataBase->size() << "\n";
    BMM_DEBUG() << "Cache size: " << m_dataBase->size() << "\n";
    BMM_DEBUG() << "-------------------------";

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

    // Get missing features from the database
    if (!cacheMissingIds->empty())
        m_dataBase->getFeatures(cacheMissingIds, features);

    // TODO: can we prevent having to iterate like this?
    // The database should be able to store the complete id so we don't have to do this?
    for (const auto& f : *features)
    {
        f->id(toValidId(f->id().featureId()));
    }

    return features;
}

bool FeatureStore::load(const std::string& indexPath)
{
    bool dataOk = m_dataBase->load(indexPath + ".data");
    bool indexOk = m_index->load(indexPath + ".index");

    return dataOk && indexOk;
}

void FeatureStore::save(const std::string& indexPath)
{
    m_dataBase->save(indexPath + ".data");
    m_index->save(indexPath + ".index");
}

bool FeatureStore::verifyIndex() const
{
    auto featureIds = m_index->queryAll();
    auto features = m_dataBase->getAllFeatures();
    size_t indexSize = featureIds->size();
    size_t dataSize = features->size();

    if (indexSize != dataSize)
    {
        // Not the same size
        return false;
    }

    for (const auto f : *features)
    {
        if (!featureIds->contains(f->id().featureId()))
        {
            // A feature id is missing
            return false;
        }
    }

    return true;
}

Id FeatureStore::toValidId(const FeatureId& featureId)
{
    return Id(m_dataSetId, featureId);
}
