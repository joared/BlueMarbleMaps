#ifndef BLUEMARBLE_TOOL
#define BLUEMARBLE_TOOL

#include "BlueMarbleMaps/Event/EventHandler.h"
#include "BlueMarbleMaps/Core/Map.h"
#include "BlueMarbleMaps/Core/MapControl.h"

#include <memory>
#include <vector>

namespace BlueMarble
{

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

            virtual void onConnected(const MapControlPtr& control, const MapPtr& map) = 0;
            virtual void onDisconnected() = 0;

            void addInteractionHandler(InteractionHandlerPtr handler);
            void removeInteractionHandler(InteractionHandlerPtr handler);

        private:
            virtual bool onEvent(const Event& event) override final;

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
