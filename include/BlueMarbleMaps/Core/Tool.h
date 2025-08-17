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

    // Abstract class for handling interactions/events
    class InteractionHandler : public EventHandler
    {
        public:
            virtual ~InteractionHandler() = default;
            virtual bool isActive() = 0;
    };
    typedef std::shared_ptr<InteractionHandler> InteractionHandlerPtr;

    // Abstract class for a tool
    class Tool : public InteractionHandler, public EventDispatcher
    {
        public:
            Tool();
            virtual ~Tool() = default;

            void onToolConnected(const MapControlPtr& control, const MapPtr& map) 
            {
                onConnected(control, map);
                for (auto& handler : m_interactionHandlers)
                {
                    if (auto tool = std::dynamic_pointer_cast<Tool>(handler))
                    {
                        tool->onConnected(control, map);
                    }
                }
            }
            void onToolDisconnected()
            {
                onDisconnected();
                for (auto& handler : m_interactionHandlers)
                {
                    if (auto tool = std::dynamic_pointer_cast<Tool>(handler))
                    {
                        tool->onDisconnected();
                    }
                }
            }

            void addInteractionHandler(InteractionHandlerPtr handler);
            void removeInteractionHandler(InteractionHandlerPtr handler);

        protected:
            virtual void onConnected(const MapControlPtr& control, const MapPtr& map) = 0;
            virtual void onDisconnected() = 0;
            virtual bool onEvent(const Event& event) override;
        private:
            std::vector<InteractionHandlerPtr> m_interactionHandlers;
            InteractionHandlerPtr              m_activeHandler;
    };
    typedef std::shared_ptr<Tool> ToolPtr;

    class BindingInteractionHandler : public InteractionHandler
    {
        public:
        private:
    };
}

#endif /* BLUEMARBLE_TOOL */
