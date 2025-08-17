#include "BlueMarbleMaps/Core/Tool.h"
#include "BlueMarbleMaps/Logging/Logging.h"

using namespace BlueMarble;

Tool::Tool()
    : m_interactionHandlers()
    , m_activeHandler(nullptr)
{
}

void Tool::addInteractionHandler(InteractionHandlerPtr handler)
{
    m_interactionHandlers.push_back(handler);
}

void Tool::removeInteractionHandler(InteractionHandlerPtr handler)
{
    for (auto it=m_interactionHandlers.begin(); it!=m_interactionHandlers.end(); it++)
    {
        if (handler == *it)
        {
            m_interactionHandlers.erase(it);
            return;
        }
    }

    BMM_DEBUG() << "Tried to remove an interaction handler that does not exist!\n";
    throw std::exception();
}

bool Tool::onEvent(const Event& event)
{
    bool handled = false;

    // First dispatch event to active handler
    auto activeHandler = m_activeHandler; // Keep a copy of the handler
    if (activeHandler)
    {
        handled = dispatchEventTo(event, activeHandler.get());
        if (activeHandler->isActive())
        {
            BMM_DEBUG() << "Interaction handler still active\n";
            // The active handler is still active. 
            // Note: the handler might not have handled the event, but is still active
            return handled;
            
        }
        
        // The active handler is not active anymore, reset pointer
        m_activeHandler = nullptr;
        BMM_DEBUG() << "Interaction handler deactivated\n";
        if (handled)
        {
            // Even if not active, it might have handled the event. Return here if so.
            return true;
        }
    }

    // Event has not been handled yet. Dispatch the event to other handlers
    for (const auto& s : m_interactionHandlers)
    {
        if (s == activeHandler)
        {
            // Active handler has already received the event and has not handled it (no longer active)
            // So we don't send it again
            continue;
        }
        
        if (!dispatchEventTo(event, s.get()))
        {
            // A handler is not allowed to say it is active if it didnt handle the event
            assert(!s->isActive());
            continue;
        }

        BMM_DEBUG() << "Interaction handler handled event\n";
        if (s->isActive())
        {
            BMM_DEBUG() << "Interaction handler activated!\n";
            m_activeHandler = s;
            return true;
        }
    }

    // No one handled it, handle it ourselfs.
    return EventHandler::onEvent(event);
}