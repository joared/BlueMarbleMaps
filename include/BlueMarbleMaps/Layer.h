#ifndef BLUEMARBLE_LAYER
#define BLUEMARBLE_LAYER

#include "Feature.h"
#include "UpdateInterfaces.h"
#include "EngineObject.h"
#include "Visualizer.h"
#include "Effect.h"

#include <vector>

namespace BlueMarble
{
    class DataSet; // Forward declaration.
    class Map;     // Forward declaration.
    
    class Layer 
        : public EngineObject
        , public IUpdateHandler
        , public FeatureHandler
    {
        public:
            Layer(bool createdefaultVisualizers = true);
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
            
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override;
            void onFeatureInput(Map& map, const std::vector<FeaturePtr>& features) override;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override;
            FeaturePtr onGetFeatureRequest(const Id& id) override;

            std::vector<VisualizerPtr>& visualizers() { return m_visualizers; }
            std::vector<VisualizerPtr>& hoverVisualizers() { return m_hoverVisualizers; }
            std::vector<VisualizerPtr>& selectionVisualizers() { return m_selectionVisualizers; }

            std::vector<EffectPtr>& effects() { return m_effects; }

        private:
            void toScreen(Map& map, const std::vector<FeaturePtr>& features, std::vector<FeaturePtr>& screenFeatures);

            void createDefaultVisualizers();
            
            bool    m_enabled;
            bool    m_selectable;
            bool    m_enabledDuringQuickUpdates;
            double  m_maxScale;
            double  m_minScale;

            std::vector<VisualizerPtr> m_visualizers;
            std::vector<VisualizerPtr> m_hoverVisualizers;
            std::vector<VisualizerPtr> m_selectionVisualizers;

            std::vector<EffectPtr> m_effects;
            DrawablePtr m_drawable; // Needed if we need to draw on our own buffer
    };
    typedef std::shared_ptr<Layer> LayerPtr;

}

#endif /* BLUEMARBLE_LAYER */
