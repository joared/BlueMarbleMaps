#include "BlueMarbleMaps/Core/DataSets/MemoryDataSet.h"

using namespace BlueMarble;

MemoryDataSet::MemoryDataSet()
    : DataSet()
    , FeatureEventPublisher(false)
    , m_features()
    , m_idToFeatureAnimation()
{
}

FeaturePtr MemoryDataSet::getFeature(const Id &id)
{
    for (auto& f : m_features)
    {
        if (f->id() == id)
            return f;
    }

    return nullptr;
}

void MemoryDataSet::startFeatureAnimation(FeaturePtr feature)
{
    assert(feature->id().dataSetId() == dataSetId());
    auto from = Point(-179, 38);
    auto to = Point(179, 31);
    startFeatureAnimation(feature, from, to);
}

void MemoryDataSet::startFeatureAnimation(FeaturePtr feature, const Point& from, const Point& to)
{
    assert(feature->id().dataSetId() == dataSetId());
    auto animation = std::make_shared<SinusoidalFeatureAnimation>(feature, from, to);
    m_idToFeatureAnimation[feature->id()] = animation;
}

// Currently only for triggering the onFeatureUpdated event
void MemoryDataSet::triggerFeatureUpdated(const FeaturePtr& feature)
{
    assert(feature->id().dataSetId() == dataSetId());
    if (featureEventsEnabled())
    {
        sendOnFeatureUpdated(feature);
    }
}

void MemoryDataSet::init()
{
}


void MemoryDataSet::addFeature(FeaturePtr feature)
{
    if (feature->id().dataSetId() != dataSetId())
    {
        feature = feature->clone();
        feature->id(generateId());
    }

    m_features.add(feature);

    if (featureEventsEnabled())
        sendOnFeatureCreated(feature);
}


void MemoryDataSet::removeFeature(const Id &id)
{
    assert(id.dataSetId() == dataSetId());

    m_features.remove(id);

    if (featureEventsEnabled())
        sendOnFeatureDeleted(id);
}

void MemoryDataSet::clear()
{
    m_features.getVector().clear();
}

FeatureEnumeratorPtr MemoryDataSet::getFeatures(const FeatureQuery& featureQuery)
{
    auto features = std::make_shared<FeatureEnumerator>();
    // TODO: feature animations should be performed before the OnUpdating event, not during rendering
    int timeStampMs = featureQuery.updateAttributes()->get<int>(UpdateAttributeKeys::UpdateTimeMs);
    for (const auto& it : m_idToFeatureAnimation)
    {
        auto& animation = it.second;
        animation->updateTimeStamp(timeStampMs);
        if (animation->isFinished())
        {
            std::cout << "Feature animation finished (" << animation->feature()->id().toString() << ")\n";
            m_idToFeatureAnimation.erase(it.first);
        }
        featureQuery.updateAttributes()->set(UpdateAttributeKeys::UpdateRequired, true);
    }

    for (auto f : m_features)
    {
        if (f->bounds().overlap(featureQuery.area()))
        {
            features->add(f);
        }
    }

    return features;
}

FeatureEventPublisher::FeatureEventPublisher(bool eventsEnabled)
{
    featureEventsEnabled(eventsEnabled);
}

void FeatureEventPublisher::addFeatureEventListener(IFeatureEventListener *listener, const Id &id)
{
    assert(featureEventsEnabled());

    auto it = m_listeners.find(id);
    if (it == m_listeners.end())
    {
        // No listeners exist. Create a new vector and add the listener
        m_listeners[id] = std::vector<IFeatureEventListener*>{listener};
        return;
    }

    // Listeners exist. Make sure it is not a double registration
    auto& listeners = it->second;
    for (auto l : listeners)
    {
        assert(l != listener);
    }

    // All ok. Add the listener
    listeners.push_back(listener);
}

void FeatureEventPublisher::removeFeatureEventListener(IFeatureEventListener* listener, const Id& id)
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(id);
    assert(it != m_listeners.end());

    auto& listeners = it->second;
    for (auto it2=listeners.begin(); it2!=listeners.end(); it2++)
    {
        if (*it2 == listener)
        {
            listeners.erase(it2);
            return;
        }
    }

    // Listener not found.
    std::cout << "FeatureEventPublisher::removeFeatureEventListener() called for a listener that is not registered.\n";
    throw std::exception();
}


bool FeatureEventPublisher::featureEventsEnabled()
{
    return m_eventsEnabled;
}


void FeatureEventPublisher::featureEventsEnabled(bool enabled)
{
    m_eventsEnabled = enabled;
}


void FeatureEventPublisher::sendOnFeatureCreated(const FeaturePtr& feature) const
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(feature->id());
    if (it == m_listeners.end())
        return;

    auto& listeners = it->second;
    for (auto listener : listeners)
    {
        listener->onFeatureCreated(feature);
    }
}

void FeatureEventPublisher::sendOnFeatureUpdated(const FeaturePtr& feature) const
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(feature->id());
    if (it == m_listeners.end())
        return;

    auto& listeners = it->second;
    for (auto listener : listeners)
    {
        listener->onFeatureUpdated(feature);
    }
}

void FeatureEventPublisher::sendOnFeatureDeleted(const Id& id) const
{
    assert(m_eventsEnabled);
    auto it = m_listeners.find(id);
    if (it == m_listeners.end())
        return;

    auto& listeners = it->second;
    for (auto listener : listeners)
    {
        listener->onFeatureDeleted(id);
    }
}