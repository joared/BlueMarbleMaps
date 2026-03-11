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
        virtual FeatureEnumeratorPtr prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) override;
        virtual void update(const MapPtr& map, const FeatureEnumeratorPtr& features, const FeatureQuery& featureQuery) override;
        virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) override final;

        void addLayer(const LayerPtr& layer) { m_subLayers.push_back(layer); }
        const std::vector<LayerPtr>& layers() const { return m_subLayers; }
        std::vector<LayerPtr>& layers() { return m_subLayers; }

        virtual void flushCache() override;
    private:
        std::vector<LayerPtr>               m_subLayers;
    };
    using LayerSetPtr = std::shared_ptr<LayerSet>;
}

#endif /* BLUEMARBLE_LAYERSET */