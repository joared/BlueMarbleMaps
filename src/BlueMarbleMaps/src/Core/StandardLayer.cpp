#include "BlueMarbleMaps/Core/StandardLayer.h"

#include <chrono>
#include <future>


using namespace BlueMarble;

StandardLayer::StandardLayer(bool createdefaultVisualizers)
    : Layer(createdefaultVisualizers)
    , m_readAsync(false)
    , m_dataSets()
    , m_cache(std::make_shared<FIFOCache>())
    , m_doRead(false)
    , m_stop(false)
{
}

StandardLayer::~StandardLayer()
{
    stopBackgroundReadingThread();
}

void StandardLayer::asyncRead(bool async)
{
    if (m_readAsync && !async)
    {
        stopBackgroundReadingThread();
    }
    else if (!m_readAsync && async)
    {
        startbackgroundReadingThread();
    }

    m_readAsync = async;
}

bool BlueMarble::StandardLayer::asyncRead()
{
    return m_readAsync;
}

void StandardLayer::addDataSet(const DataSetPtr &dataSet)
{
    return m_dataSets.push_back(dataSet);
}

FeatureEnumeratorPtr StandardLayer::update(const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    auto enumerator = std::make_shared<FeatureEnumerator>();
    auto features = std::make_shared<FeatureCollection>();
    enumerator->setFeatures(features);

    if (!isActiveForQuery(featureQuery))
    {
        return enumerator;
    }

    if (m_readAsync)
    {
        {
            std::lock_guard lock(m_mutex);
            auto cachedFeatures = m_cache->getAllFeatures();
            for (const auto& f : *cachedFeatures)
            {
                if (f->bounds().overlap(featureQuery.area()))
                {
                    features->add(f);
                }
            }
            m_doRead = true;
            m_query = featureQuery;
            m_crs = crs;
        }
        m_cond.notify_one();
    }
    else
    {
        return getFeatures(crs, featureQuery, true);
    }

    return enumerator;
}

FeatureEnumeratorPtr StandardLayer::getFeatures(const CrsPtr &crs, const FeatureQuery& featureQuery, bool activeLayersOnly)
{
    auto features = std::make_shared<FeatureEnumerator>();
    if (activeLayersOnly)
    {
        if (featureQuery.scale() > maxScale())
            return features;
        if (featureQuery.scale() < minScale())
            return features;
        // if (featureQuery.quickUpdateEnabled() && !enabledDuringQuickUpdates())
        //     return features;
        if (!enabled())
        {
            return features;
        }
    }
    
    for (const auto& d : m_dataSets)
    {
        auto dataSetFeatures = d->getFeatures(featureQuery);
        features->addEnumerator(dataSetFeatures);
    }

    return features;
}

void StandardLayer::backgroundReadingThread()
{
    BMM_DEBUG() << "StandardLayer::backgroundReadingThread() Started background reading thread\n";
    
    while (true)
    {
        std::unique_lock lock(m_mutex);
        m_cond.wait(lock, [this](){ return m_doRead || m_stop; });
        if (m_stop) break;

        m_doRead = false;
        auto crs = m_crs;
        auto query = m_query;
        lock.unlock();

        BMM_DEBUG() << "StandardLayer::backgroundReadingThread() LOAD for some time\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Faking load
        BMM_DEBUG() << "StandardLayer::backgroundReadingThread() LOAD done!\n";
        auto features = getFeatures(crs, query, true);
        
        std::unique_lock lock2(m_mutex);
        while (features->moveNext())
        {
            const auto& f = features->current();
            m_cache->insert(f->id(), f);
        }
        lock2.unlock();
        
        m_cond.notify_one();
    }

    BMM_DEBUG() << "StandardLayer::backgroundReadingThread() Background reading thread exited\n";
}

void StandardLayer::startbackgroundReadingThread()
{
    m_readingThread = std::thread([this](){backgroundReadingThread();});
}

void StandardLayer::stopBackgroundReadingThread()
{
    {
        std::lock_guard lock(m_mutex);
        m_stop = true;
    }
    m_cond.notify_one();
    if (m_readingThread.joinable())
    {
        m_readingThread.join();
    }
}
