#include "BlueMarbleMaps/Core/UpdateInterfaces.h"
#include "BlueMarbleMaps/Core/EngineObject.h"

using namespace BlueMarble;

// void UpdateRequestPropagator::addUpdateHandler(IUpdateHandler* handler)
// {
// }

// void UpdateRequestPropagator::sendUpdateRequest(Map &map, const Rectangle &updateArea)
// {
// }

// void FeatureProvider::addFeatureHandler(IFeatureHandler *handler)
// {
// }

// void FeatureProvider::sendFeatures(const std::vector<FeaturePtr> &features)
// {
// }

FeatureEnumerator::FeatureEnumerator()
    : m_iteratorIndex(-1)
    , m_iterationIndex(-1)
    , m_features()
    , m_subEnumerators()
{
    reset();
}


const FeaturePtr &FeatureEnumerator::current() const
{
    if (m_iterationIndex == -1)
    {
        BMM_DEBUG() << "Iterator index not initialized with moveNext\n";
        throw std::exception();
    }

    if (m_iteratorIndex == -1)
    {
        // Our own features
        if (m_iterationIndex >= m_features.size())
        {
            BMM_DEBUG() << "Something went wrong during iteration index, out of bounds...\n";
            throw std::exception();
        }
        return m_features[m_iterationIndex];
    }

    // Other iterator
    if (m_iteratorIndex >= m_subEnumerators.size())
    {
        BMM_DEBUG() << "Something went wrong during iterator index, out of bounds...\n";
        throw std::exception();
    }

    return m_subEnumerators[m_iteratorIndex]->current();
}

bool FeatureEnumerator::moveNext()
{
    if (m_iteratorIndex == -1)
    {
        // Our own
        m_iterationIndex++;
        if (m_iterationIndex < m_features.size())
        {
            return true;
        }
        else
        {
            m_iteratorIndex = 0; // switch to sub enumerators
        }
    }

    // Other iterator
    if (m_iteratorIndex < m_subEnumerators.size())
    {
        if (m_subEnumerators[m_iteratorIndex]->moveNext())
        {
            return true;
        }
        else
        {
            // increment and try again
            m_iteratorIndex++;
            return moveNext();
        }
    }

    return false;
}

void FeatureEnumerator::reset()
{
    m_iteratorIndex = -1;
    m_iterationIndex = -1;

    for (const auto& e : m_subEnumerators)
    {
        e->reset();
    }
}

void FeatureEnumerator::add(const FeaturePtr &feature)
{
    m_features.push_back(feature);
}

int FeatureEnumerator::size()
{
    int s = m_features.size();

    for (const auto& e : m_subEnumerators)
    {
        s += e->size();
    }

    return s;
}

FeatureHandler::FeatureHandler()
    : m_updateHandlers()
{
}

void FeatureHandler::addUpdateHandler(IUpdateHandler *handler)
{
    m_updateHandlers.push_back(handler);
    
    // FIXME: uggly fix, remove from here
    auto obj = dynamic_cast<EngineObject*>(this);
    auto child = dynamic_cast<EngineObject*>(handler);
    if (obj && child)
    {
        obj->addChild(child);
    }
    else
    { 
        std::cout << "handler must inherit EngineObject\n";
        throw std::exception();
    }
}

void FeatureHandler::sendUpdateRequest(Map &map, const Rectangle &updateArea)
{
    for (auto handler : m_updateHandlers)
    {
        handler->onUpdateRequest(map, updateArea, this);
    }
}

void FeatureHandler::sendGetFeaturesRequest(const Attributes &attributes, std::vector<FeaturePtr>& features)
{
    for (auto handler : m_updateHandlers)
    {
        handler->onGetFeaturesRequest(attributes, features);
    }
}

FeaturePtr FeatureHandler::sendGetFeatureRequest(const Id &id)
{
    for (auto handler : m_updateHandlers)
    {
        if (auto f = handler->onGetFeatureRequest(id))
        {
            return f;
        }
    }

    return nullptr;
}
