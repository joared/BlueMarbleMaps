#include "BlueMarbleMaps/Core/Layer/Layer.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/DataSets/DataSet.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace BlueMarble;

Layer::Layer()
    : m_enabled(true)
    , m_selectable(true) // TODO: use this for something
    , m_enabledDuringQuickUpdates(true)
    , m_maxScale(std::numeric_limits<double>::infinity())
    , m_minScale(0)
    , m_renderingEnabled(true)
    , m_drawable(nullptr)
{
    
}


void Layer::enabled(bool enabled)
{
    m_enabled = enabled;
}


bool Layer::enabled() const
{
    return m_enabled;
}

void Layer::selectable(bool selectable)
{
    m_selectable = selectable;
}

bool Layer::selectable()
{
    return m_selectable;
}

bool Layer::isActiveForQuery(const FeatureQuery& featureQuery)
{
    if (featureQuery.scale() > maxScale())
        return false;
    if (featureQuery.scale() < minScale())
        return false;
    // if (featureQuery.quickUpdateEnabled() && !enabledDuringQuickUpdates())
    //     return features;
    if (!enabled())
    {
        return false;
    }

    return true;
}
