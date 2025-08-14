#ifndef BLUEMARBLE_MAPCONTROL
#define BLUEMARBLE_MAPCONTROL

#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Event/EventManager.h"
#include "BlueMarbleMaps/Core/Core.h"
#include "BlueMarbleMaps/Core/Tool.h"

namespace BlueMarble
{
    class MapControl 
        : public EventManager // TODO: could this sort of be private, as well as EventDispatcher?
        , public std::enable_shared_from_this<MapControl>
    {
        public:
            MapControl();
            virtual ~MapControl() = default;
            static int64_t getGinotonicTimeStampMs() { return getTimeStampMs(); };
            virtual void* getWindow() = 0;
            
            void setView(MapPtr mapView);
            MapPtr getView();

            void setTool(const ToolPtr& tool);

            void updateView();
            bool updateRequired();
            void updateViewInternal(); // TODO: make private

            // Override resize of EventManager
            bool resize(int width, int height, int64_t timeStampMs) override final;
        protected:
            void handleResize(int width, int height);
        private:
            MapPtr  m_mapView;
            ToolPtr m_tool;
            bool    m_updateRequired;

    };
    typedef std::shared_ptr<MapControl> MapControlPtr;
}

#endif /* BLUEMARBLE_MAPCONTROL */
