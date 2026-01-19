#include "BlueMarbleMaps/Core/Index/FeatureStore.h"
#include <fstream>
#include <unordered_set>


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

FeaturePtr FeatureStore::getFeature(const FeatureId& id)
{
    auto featureIds = std::make_shared<FeatureIdCollection>();
    featureIds->add(id);
    auto features = getFeatures(featureIds);
    assert(features->size() == 1);

    return features->get(0);
}

FeatureCollectionPtr FeatureStore::getFeatures(const FeatureIdCollectionPtr& featureIds)
{
    // TODO: maybe store a member collection with preallocated size
    auto features = std::make_shared<FeatureCollection>();
    features->reserve(featureIds->size());

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
        for (const auto& f : *nonCachedFeatures)
        {
            f->id(Id(m_dataSetId, f->id().featureId())); // The database has no idea about the dataset id, but we do!
            if (m_cache)
                m_cache->insert(f->id(), f);
        }

        // Add the noncached features to the result
        features->addRange(*nonCachedFeatures);

        // Some debugging
        // BMM_DEBUG() << "FeatureStore::getFeatures()\n";
        // BMM_DEBUG() << "Queried " << nonCachedFeatures->size() << " new features from database\n";
        // BMM_DEBUG() << "Database size: " << m_dataBase->size() << "\n";
        // if (m_cache)
        //     BMM_DEBUG() << "Cache size: " << m_cache->size() << "\n";
        // BMM_DEBUG() << "-------------------------\n";
    }

    return features;
}

FeatureIdCollectionPtr FeatureStore::queryIds(const Rectangle& area)
{
    return m_index->query(area);
}

FeatureCollectionPtr FeatureStore::query(const Rectangle& area, const FeatureIdCollectionPtr& featureIds)
{
    auto queriedIds = queryIds(area);
    if (featureIds && !featureIds->empty())
    {
        int sizeQueried = queriedIds->size();
        int sizeRequested = featureIds->size();
        queriedIds = idIntersection(featureIds, queriedIds);
        BMM_DEBUG() << "FeatureStore::query() Queried #" << sizeQueried << ", requested #" << sizeRequested << ", got #" << queriedIds->size() << "\n";
    }
    return getFeatures(queriedIds);
}

bool FeatureStore::load(const std::string& indexPath)
{
    // We never rebuild database on load, it has to be done eplicitly since it takes time
    BMM_DEBUG() << "------- FeatureStore::load() -------\n";
    BMM_DEBUG() << "------- Database loading -------\n";
    auto peristableDb = dynamic_cast<IPersistable*>(m_dataBase.get());
    if (peristableDb && !peristableDb->load(getDatabasePersistanceContext(peristableDb, indexPath)))
    {
        return false;
    }

    auto persistableIndex = dynamic_cast<IPersistable*>(m_index.get());
    bool indexLoaded = false;
    if (persistableIndex)
    {
        BMM_DEBUG() << "------- Index loading -------\n";
        indexLoaded = persistableIndex->load(getIndexPersistanceContext(persistableIndex, indexPath));
    }
    
    // We allow to rebuild the index on load
    if (!indexLoaded)
    {
        BMM_DEBUG() << "------- Index building -------\n";
        BMM_DEBUG() << "------- Query all features from database -------\n";
        auto features = m_dataBase->getAllFeatures();
        BMM_DEBUG() << "------- Building index -------\n";
        m_index->build(features);
        if (persistableIndex)
        {
            BMM_DEBUG() << "------- Saving index -------\n";
            persistableIndex->save(getIndexPersistanceContext(persistableIndex, indexPath));
        }
    }
    BMM_DEBUG() << "------- FeatureStore::load() complete -------\n";

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

    // Verify id match
    std::unordered_map<FeatureId, int> idCount1;
    std::unordered_map<FeatureId, int> idCount2;
    for (const auto& f : *features)
    {
        auto fid = f->id().featureId();
        auto it = idCount1.find(fid);
        if (it == idCount1.end())
        {
            idCount1[fid] = 1;
        }
        else
        {
            idCount1[fid] += 1;
        }
    }

    for (const auto fid : *featureIds)
    {
        auto it = idCount2.find(fid);
        if (it == idCount2.end())
        {
            idCount2[fid] = 1;
        }
        else
        {
            idCount2[fid] += 1;
        }
    }

    for (const auto& pair: idCount1)
    {
        FeatureId fid = pair.first;
        int count1 = pair.second;
        
        auto it = idCount2.find(fid);
        if (it == idCount2.end())
        {
            BMM_DEBUG() << "Feature id missing in index: " << fid << "\n";
            return false;
        }
        int count2 = it->second;
        if (count1 != count2)
        {
            BMM_DEBUG() << "Feature id count missmatch: " << count1 << " != " << count2 << "\n";
            return false;
        }
        idCount2.erase(fid);
    }

    if (!idCount2.empty())
    {
        BMM_DEBUG() << "Index has " << idCount2.size() << " more ids than database\n";
        return false;
    }

    return true;
}

void FeatureStore::flushCache()
{
    if (m_cache)
        m_cache->clear();
}

IPersistable::PersistanceContext BlueMarble::FeatureStore::getIndexPersistanceContext(IPersistable* p, const std::string& indexPath)
{
    return IPersistable::PersistanceContext
    {
        indexPath + "._" + p->persistanceId() + "_index"
    };
}

IPersistable::PersistanceContext BlueMarble::FeatureStore::getDatabasePersistanceContext(IPersistable* p, const std::string& indexPath)
{
    return IPersistable::PersistanceContext
    {
        indexPath + "._" + p->persistanceId() + "_database"
    };
}

void FeatureStore::build(const FeatureCollectionPtr& features, const std::string& indexPath)
{
    for (const auto& feature : *features)
    {
        // We can only add features associated with one data set
        assert(feature->id().dataSetId() == m_dataSetId);
    }

    m_dataBase->build(features);
    m_index->build(features);

    auto peristableDb = dynamic_cast<IPersistable*>(m_dataBase.get());
    auto peristableIndex = dynamic_cast<IPersistable*>(m_index.get());
    if (peristableDb) peristableDb->save(getDatabasePersistanceContext(peristableDb, indexPath));
    if (peristableIndex) peristableIndex->save(getIndexPersistanceContext(peristableIndex, indexPath));
}


Id FeatureStore::toValidId(const FeatureId& featureId)
{
    return Id(m_dataSetId, featureId);
}

FeatureIdCollectionPtr FeatureStore::idIntersection(const FeatureIdCollectionPtr& requested, const FeatureIdCollectionPtr& candidates)
{
    // 1. Use smaller list for hash set (minimize memory)
    const auto& smaller = requested->size() <= candidates->size() ? requested : candidates;
    const auto& larger  = requested->size() >  candidates->size() ? requested : candidates;

    // 2. Build hash set from smaller list
    std::unordered_set<FeatureId> set;
    set.reserve(smaller->size());  // ← critical for performance
    set.insert(smaller->begin(), smaller->end());

    // 3. Scan larger list, keep matches
    auto result = std::make_shared<FeatureIdCollection>();
    result->reserve(std::min(requested->size(), candidates->size()));  // optimistic

    for (auto id : *larger) 
    {
        auto it = set.find(id);
        if (it != set.end()) 
        {
            result->add(*it);
        }
    }

    return result;
}
