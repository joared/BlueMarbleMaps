#include "BlueMarbleMaps/Core/Index/MemoryDatabase.h"

using namespace BlueMarble;

MemoryDatabase::MemoryDatabase()
    : m_cache()
{
}

FeaturePtr MemoryDatabase::getFeature(const FeatureId& featureId)
{
    // In a database, only feature id is relevant. We use "0" as dataset id
    auto id = Id(0, featureId);

    return m_cache.getFeature(id);
}

FeatureCollectionPtr MemoryDatabase::getFeatures(const FeatureIdCollectionPtr &ids)
{
    auto features = std::make_shared<FeatureCollection>();

    getFeatures(ids, features);

    return features;
}

void MemoryDatabase::getFeatures(const FeatureIdCollectionPtr &ids, FeatureCollectionPtr& featuresOut)
{
    for (const auto& id : *ids)
    {
        featuresOut->add(getFeature(id));
    }
}

FeatureCollectionPtr MemoryDatabase::getAllFeatures()
{
    return m_cache.getAllFeatures();
}

void MemoryDatabase::removeFeature(const FeatureId& featureId)
{
    // In a database, only feature id is relevant. We use "0" as dataset id
    auto id = Id(0, featureId);   

    m_cache.remove(id);
}

size_t MemoryDatabase::size() const
{
    return m_cache.size();
}

void MemoryDatabase::save(const std::string &path) const
{
    // Do nothing
}

bool MemoryDatabase::load(const std::string &path)
{
    // We don't support saving/loading for now
    return false;
}

bool MemoryDatabase::build(const FeatureCollectionPtr& features, const std::string& path)
{
    for (const auto& feature : * features)
    {
        // In a database, only feature id is relevant. We use "0" as dataset id
        auto id = Id(0, feature->id().featureId());

        if (m_cache.contains(id))
        {
            BMM_DEBUG() << "WARNING: MemoryDatabase::addFeature() Id already exist in the database!: " << id.toString() << "\n";
        }
        m_cache.insert(id, feature);
    }

    return true;
}