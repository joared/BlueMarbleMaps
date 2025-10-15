#ifndef BLUEMARBLE_LAYERSET
#define BLUEMARBLE_LAYERSET

#include "BlueMarbleMaps/Core/Layer/Layer.h"

namespace BlueMarble
{
    class DataSet; // Forward declaration.
    class Map;     // Forward declaration.
    typedef std::shared_ptr<Map> MapPtr;
    
    class LayerSet
        : public Layer
    {
    public:
        LayerSet();
        virtual ~LayerSet() = default;

        virtual void hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects) override final;
        virtual void prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override final;
        virtual void update(const MapPtr &crs) override final;
        virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override final;
        
    private:
        std::vector<LayerPtr> m_subLayers;
    };
}

#endif /* BLUEMARBLE_LAYERSET */