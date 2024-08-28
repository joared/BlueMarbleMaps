#ifndef BLUEMARBLE_LAYER
#define BLUEMARBLE_LAYER

#include "Feature.h"
#include "UpdateInterfaces.h"

#include <vector>

namespace BlueMarble
{
    class DataSet; // Forward declaration.
    class Map;     // Forward declaration.
    
    class Layer 
        : public IUpdateHandler
        , public FeatureHandler
    {
        public:
            Layer();
            void enabled(bool enabled);
            bool enabled() const;
            double maxScale() { return m_maxScale; }
            void maxScale(double maxScale) { m_maxScale = maxScale; }
            double minScale() {return m_minScale; }
            void minScale(double minScale) { m_minScale = minScale; }
            
            void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) override;
            void onFeatureInput(Map& map, const std::vector<FeaturePtr>& features) override;
            void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) override;
            FeaturePtr onGetFeatureRequest(const Id& id) override;

            void renderPoint(Map& map, FeaturePtr feature);
            void renderLine(Map& map, FeaturePtr feature);
            void renderPolygon(Map& map, FeaturePtr feature);
            void renderMultiPolygon(Map& map, FeaturePtr feature);
            void renderRaster(Map& map, FeaturePtr feature);

            void toScreen(Map& map, const std::vector<FeaturePtr>& features, std::vector<FeaturePtr>& screenFeatures);
            
        private:
            bool    m_enabled;
            double  m_maxScale;
            double  m_minScale;
    };

}

#endif /* BLUEMARBLE_LAYER */
