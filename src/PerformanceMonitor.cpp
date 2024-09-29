#include "PerformanceMonitor.h"

using namespace BlueMarble;

PerformanceMonitor::PerformanceMonitor(Map &map)
    : m_map(map)
    , m_isRunning(false)
    , m_sampler()
{
}


void PerformanceMonitor::start()
{
    if (m_isRunning)
    {
        std::cout << "PerformanceMonitor::start() Performance monitor is already running!\n";
        throw std::exception();
    }
    m_sampler = PerformanceSampler();
    m_map.addMapEventHandler(this);
    m_isRunning = true;
    m_map.update(true); // Perform one update to get at least one sample
}


PerformanceReport PerformanceMonitor::stop()
{
    m_map.removeMapEventHandler(this);
    m_isRunning = false;

    return PerformanceReport(m_sampler);
}

PerformanceReport PerformanceMonitor::staticStartStop(int durationMs)
{
    int startTimeStamp = getTimeStampMs();
    start();
    while (getTimeStampMs() - startTimeStamp < durationMs)
    {
        m_map.update(true);
    }
    
    return stop();
}

bool PerformanceMonitor::isRunning()
{
    return m_isRunning;
}


void PerformanceMonitor::OnUpdating(Map &map)
{
    int timeStamp = getTimeStampMs();
    m_sampler.sampleStart("Update", timeStamp);
    m_sampler.sampleStart("Layers", timeStamp);
}


void PerformanceMonitor::OnCustomDraw(Map &map)
{
    int timeStamp = getTimeStampMs();
    m_sampler.sampleEnd("Layers", timeStamp);
    m_sampler.sampleStart("Custom draw", timeStamp);
}


void PerformanceMonitor::OnUpdated(Map& map)
{
    int timeStamp = getTimeStampMs();
    m_sampler.sampleEnd("Update", timeStamp);
    m_sampler.sampleEnd("Custom draw", timeStamp);
}


PerformanceSampler::PerformanceSampler()
    : m_operationNameToSampleMap()
{
}


Sample& PerformanceSampler::getSample(const std::string& sampleKey)
{
    if (m_operationNameToSampleMap.find(sampleKey) == m_operationNameToSampleMap.end())
        m_operationNameToSampleMap[sampleKey] = Sample(sampleKey);

    return m_operationNameToSampleMap[sampleKey];
}


void PerformanceSampler::sampleStart(const std::string& sampleKey, int timeStamp)
{
    getSample(sampleKey).sampleStart(timeStamp);
}


void PerformanceSampler::sampleEnd(const std::string& sampleKey, int timeStamp)
{
    getSample(sampleKey).sampleEnd(timeStamp);
}

std::vector<Sample> PerformanceSampler::getSamples()
{
    auto samples = std::vector<Sample>();
    for (auto it : m_operationNameToSampleMap)
    {
        samples.push_back(it.second);
    }

    return samples;
}

PerformanceReport::PerformanceReport(const PerformanceSampler& sampler)
    : m_sampler(sampler)
{
}

std::string PerformanceReport::toString()
{
    std::string s;
    for (auto& sample : m_sampler.getSamples())
    {
        s += sample.toString() + "\n";
    }

    return s;
}
