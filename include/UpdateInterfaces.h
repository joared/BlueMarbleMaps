#ifndef BLUEMARBLE_UPDATEINTERFACES
#define BLUEMARBLE_UPDATEINTERFACES

#include "Feature.h"

namespace BlueMarble
{
    class Map;              // Forward declaration.
    class FeatureHandler;  // Forward declaration.

    // Downstream
    // Interface for layers, operators and data sets that handles update requests in the operator chain
    class IUpdateHandler
    {
        public:
            virtual void onUpdateRequest(Map& map, const Rectangle& updateArea, FeatureHandler* handler) = 0;
            virtual void onGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features) = 0;
            virtual FeaturePtr onGetFeatureRequest(const Id& id) = 0;
            virtual ~IUpdateHandler() = default;
    };

    // Upstream
    // Interface for layers, operators and data sets that handles features as a results of and update request
    class FeatureHandler
    {
        public:
            FeatureHandler();
            // TODO: we send "back" a reference to the map since layers needs it to render
            virtual void onFeatureInput(Map &map, const std::vector<FeaturePtr>& features) = 0;
            virtual ~FeatureHandler() = default;
            void addUpdateHandler(IUpdateHandler* handler);
        protected:
            void sendUpdateRequest(Map& map, const Rectangle& updateArea);
            void sendGetFeaturesRequest(const Attributes& attributes, std::vector<FeaturePtr>& features);
            FeaturePtr sendGetFeatureRequest(const Id& id);
        private:
            std::vector<IUpdateHandler*> m_updateHandlers;
    };
}

#endif /* BLUEMARBLE_UPDATEINTERFACES */
