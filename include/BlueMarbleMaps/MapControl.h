#ifndef BLUEMARBLE_MAPCONTROL
#define BLUEMARBLE_MAPCONTROL

#include "Map.h"
#include "EventHandler.h"
#include "Core.h"

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
        protected:
            void handleResize(int width, int height);
        private:
            bool m_updateRequired;
            MapPtr m_mapView;

    };
    typedef std::shared_ptr<MapControl> MapControlPtr;
}

#endif /* BLUEMARBLE_MAPCONTROL */
