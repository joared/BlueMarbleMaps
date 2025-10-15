#include "BlueMarbleMaps/Core/Layer/LayerSet.h"

using namespace BlueMarble;

LayerSet::LayerSet()
    : Layer()
{
}

void LayerSet::hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects)
{
    for (const auto& l : m_subLayers)
    {
        l->hitTest(map, bounds, presObjects);
    }
}

void LayerSet::prepare(const CrsPtr &crs, const FeatureQuery &featureQuery)
{
    for (const auto& l : m_subLayers)
    {
        l->prepare(crs, featureQuery);
    }
}


void LayerSet::update(const MapPtr& map)
{
    for (const auto& l : m_subLayers)
    {
        l->update(map);
    }
}


FeatureEnumeratorPtr LayerSet::getFeatures(const CrsPtr &crs, const FeatureQuery& featureQuery, bool activeLayersOnly)
{
    auto enumerator = std::make_shared<FeatureEnumerator>();
    for (const auto& l : m_subLayers)
    {
        enumerator->addEnumerator(l->getFeatures(crs, featureQuery, activeLayersOnly));
    }

    return enumerator;
}
