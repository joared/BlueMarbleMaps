#include "BlueMarbleMaps/Core/Index/FIFOCache.h"

using namespace BlueMarble;

FIFOCache::FIFOCache()
    : m_cache()
{
}

void FIFOCache::insert(const Id& id, const FeaturePtr& feature)
{
    m_cache[id] = feature;
}

void FIFOCache::remove(const Id& id)
{
    m_cache.erase(id);
}

bool FIFOCache::contains(const Id& id) const
{
    return m_cache.find(id) != m_cache.end();
}

const FeaturePtr& FIFOCache::getFeature(const Id& id) const
{
    return m_cache.at(id);
}

FeatureCollectionPtr FIFOCache::getAllFeatures() const
{
    auto features = std::make_shared<FeatureCollection>();

    for (const auto& it : m_cache)
    {
        features->add(it.second);
    }

    return features;
}

size_t BlueMarble::FIFOCache::size() const
{
    return m_cache.size();
}

void FIFOCache::clear()
{
    m_cache.clear();
}
