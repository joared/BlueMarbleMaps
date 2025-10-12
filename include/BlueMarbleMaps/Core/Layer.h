#ifndef BLUEMARBLE_LAYER
#define BLUEMARBLE_LAYER

#include "BlueMarbleMaps/Core/Feature.h"
#include "BlueMarbleMaps/Core/UpdateInterfaces.h"
#include "BlueMarbleMaps/Core/ResourceObject.h"
#include "BlueMarbleMaps/Core/Visualizer.h"
#include "BlueMarbleMaps/Core/Effect.h"

#include <vector>

namespace BlueMarble
{
    class DataSet; // Forward declaration.
    class Map;     // Forward declaration.
    typedef std::shared_ptr<Map> MapPtr;
    
    class Layer 
        : public ResourceObject
    {
        public:
            Layer(bool createdefaultVisualizers = true);
            virtual ~Layer() = default;
            void renderingEnabled(bool enabled) { m_renderingEnabled=enabled; }
            void enabled(bool enabled);
            bool enabled() const;
            void selectable(bool selectable);
            bool selectable();
            void enabledDuringQuickUpdates(bool enabled) { m_enabledDuringQuickUpdates = enabled; };
            bool enabledDuringQuickUpdates() const { return m_enabledDuringQuickUpdates; };
            double maxScale() { return m_maxScale; }
            void maxScale(double maxScale) { m_maxScale = maxScale; }
            double minScale() {return m_minScale; }
            void minScale(double minScale) { m_minScale = minScale; }

            virtual FeatureEnumeratorPtr update(const CrsPtr &crs, const FeatureQuery& featureQuery) = 0;
            virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) = 0;

            std::vector<VisualizerPtr>& visualizers() { return m_visualizers; }
            std::vector<VisualizerPtr>& hoverVisualizers() { return m_hoverVisualizers; }
            std::vector<VisualizerPtr>& selectionVisualizers() { return m_selectionVisualizers; }

            std::vector<EffectPtr>& effects() { return m_effects; }

            bool isActiveForQuery(const FeatureQuery& query);

        private:

            void createDefaultVisualizers();
            
            bool    m_enabled;
            bool    m_selectable;
            bool    m_enabledDuringQuickUpdates;
            double  m_maxScale;
            double  m_minScale;
            bool    m_renderingEnabled;

            std::vector<VisualizerPtr> m_visualizers;
            std::vector<VisualizerPtr> m_hoverVisualizers;
            std::vector<VisualizerPtr> m_selectionVisualizers;

            std::vector<EffectPtr> m_effects;
            DrawablePtr m_drawable; // Needed if we need to draw on our own buffer
    };
    typedef std::shared_ptr<Layer> LayerPtr;

}

#endif /* BLUEMARBLE_LAYER */
