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
            Layer();
            virtual ~Layer() = default;
            void renderingEnabled(bool enabled) { m_renderingEnabled=enabled; }
            bool renderingEnabled() { return m_renderingEnabled; }
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

            virtual void hitTest(const MapPtr& map, const Rectangle& bounds, std::vector<PresentationObject>& presObjects) = 0;
            virtual void prepare(const CrsPtr &crs, const FeatureQuery& featureQuery) = 0;
            virtual void update(const MapPtr& map) = 0;
            virtual FeatureEnumeratorPtr getFeatures(const CrsPtr& crs, const FeatureQuery& featureQuery, bool activeLayersOnly) = 0;
            virtual void flushCache() = 0;

            bool isActiveForQuery(const FeatureQuery& query);

        private:
            
            bool    m_enabled;
            bool    m_selectable;
            bool    m_enabledDuringQuickUpdates;
            double  m_maxScale;
            double  m_minScale;
            bool    m_renderingEnabled;

            DrawablePtr m_drawable; // Needed if we need to draw on our own buffer
    };
    typedef std::shared_ptr<Layer> LayerPtr;

}

#endif /* BLUEMARBLE_LAYER */
