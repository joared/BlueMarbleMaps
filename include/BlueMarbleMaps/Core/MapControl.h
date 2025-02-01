#ifndef BLUEMARBLE_MAPCONTROL
#define BLUEMARBLE_MAPCONTROL

#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Event/EventHandler.h"
#include "BlueMarbleMaps/Core/Core.h"

namespace BlueMarble
{
    class MapControl : public EventManager
    {
        public:
            MapControl();
            virtual ~MapControl() = default;
            static int64_t getGinotonicTimeStampMs() { return getTimeStampMs(); };
            virtual void* getWindow() = 0;
            
            void setView(MapPtr mapView);
            MapPtr getView();

            void updateView();
            bool updateRequired();
            void updateViewInternal(); // TODO: make private

            // Override resize of EventManager
            bool resize(int width, int height, int64_t timeStampMs) override final;
        protected:
            void handleResize(int width, int height);
        private:
            MapPtr m_mapView;
            bool m_updateRequired;

    };
    typedef std::shared_ptr<MapControl> MapControlPtr;
}

#endif /* BLUEMARBLE_MAPCONTROL */
