#include "BlueMarbleMaps/Core/Layer/LayerSet.h"
#include "BlueMarbleMaps/Core/Map.h"


using namespace BlueMarble;

LayerSet::LayerSet()
    : Layer()
{
}

void LayerSet::hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects)
{
    FeatureQuery featureQuery;
    featureQuery.area(bounds);
    featureQuery.scale(map->scale());
    featureQuery.updateAttributes(&map->updateAttributes());

    for (const auto& l : m_subLayers)
    {
        if (!l->selectable() ||
            !l->isActiveForQuery(featureQuery))
        {
            continue;;
        }
        
        l->hitTest(map, bounds, presObjects);
    }
}

FeatureEnumeratorPtr LayerSet::prepare(const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    auto e = std::make_shared<FeatureEnumerator>();

    if (!isActiveForQuery(featureQuery))
    {
        return e;
    }

    for (const auto& l : m_subLayers)
    {
        e->addEnumerator(l->prepare(crs, featureQuery));
    }

    return e;
}


void LayerSet::update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery)
{
    assert(features->subEnumerators().size() == layers().size());

    for (int i(0); i<features->subEnumerators().size(); ++i)
    {
        layers()[i]->update(map, features->subEnumerators()[i], featureQuery);
    }
}


FeatureEnumeratorPtr LayerSet::getFeatures(const CrsPtr &crs, const FeatureQuery& featureQuery, bool activeLayersOnly)
{
    auto enumerator = std::make_shared<FeatureEnumerator>();

    if (activeLayersOnly &&
        !isActiveForQuery(featureQuery))
    {
        return enumerator;
    }

    for (const auto& l : m_subLayers)
    {
        enumerator->addEnumerator(l->getFeatures(crs, featureQuery, activeLayersOnly));
    }

    return enumerator;
}

void LayerSet::removeLayer(const LayerPtr &layer)
{
    for (auto it = m_subLayers.begin(); it != m_subLayers.end(); ++it)
    {
        if (layer == *it)
        {
            m_subLayers.erase(it);
            return;
        }
    }

    throw std::runtime_error("LayerSet::removeLayer() layer not found");
}

void LayerSet::flushCache()
{
    for (const auto& l : m_subLayers)
    {
        l->flushCache();
    }
}
