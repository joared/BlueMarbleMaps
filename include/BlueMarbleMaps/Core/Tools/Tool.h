#ifndef BLUEMARBLE_TOOL
#define BLUEMARBLE_TOOL

#include "BlueMarbleMaps/Event/EventHandler.h"

#include <memory>
#include <vector>

namespace BlueMarble
{
    class Map;
    typedef std::shared_ptr<Map> MapPtr;
    class MapControl;
    typedef std::shared_ptr<MapControl> MapControlPtr;


    // Abstract class for a tool
    class Tool;
    typedef std::shared_ptr<Tool> ToolPtr;
    class Tool 
        : public EventHandler
    {
        public:
            Tool();
            virtual ~Tool() = default;
            virtual bool isActive() = 0;
            virtual void onConnected(const MapControlPtr& control, const MapPtr& map) = 0;
            virtual void onDisconnected() = 0;
    };
}

#endif /* BLUEMARBLE_TOOL */
