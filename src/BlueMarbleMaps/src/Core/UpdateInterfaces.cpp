#include "BlueMarbleMaps/Core/UpdateInterfaces.h"

using namespace BlueMarble;

FeatureEnumerator::FeatureEnumerator(bool isComplete)
    : m_isComplete(isComplete)
    , m_iteratorIndex(-1)
    , m_iterationIndex(-1)
    , m_features(std::make_shared<FeatureCollection>())
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
        if (m_iterationIndex >= m_features->size())
        {
            BMM_DEBUG() << "Something went wrong during iteration index, out of bounds...\n";
            throw std::exception();
        }
        return m_features->get(m_iterationIndex);
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
        if (m_iterationIndex < m_features->size())
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
    // if (m_iterationIndex == -1 && m_iteratorIndex == -1)
    // {
    //     // Already reset
    //     return;
    // }
    
    m_iteratorIndex = -1;
    m_iterationIndex = -1;

    for (const auto& e : m_subEnumerators)
    {
        e->reset();
    }
}

void FeatureEnumerator::add(const FeaturePtr &feature)
{
    m_features->add(feature);
}

void FeatureEnumerator::setFeatures(const FeatureCollectionPtr& features)
{
    m_features = features;
}

int FeatureEnumerator::size()
{
    int s = m_features->size();

    for (const auto& e : m_subEnumerators)
    {
        s += e->size();
    }

    return s;
}

bool FeatureEnumerator::isComplete() const
{
    if (!m_isComplete)
    {
        return false;
    }

    for (const auto& e : m_subEnumerators)
    {
        if (!e->isComplete())
        {
            return false;
        }
    }

    return true;
}
