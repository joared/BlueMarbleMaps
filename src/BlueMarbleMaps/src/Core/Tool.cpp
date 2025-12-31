#include "BlueMarbleMaps/Core/Tools/Tool.h"
#include "BlueMarbleMaps/Logging/Logging.h"

using namespace BlueMarble;

Tool::Tool()
    : m_subTools()
    , m_activeSubTool(nullptr)
{
}

void Tool::addSubTool(const ToolPtr& tool)
{
    m_subTools.push_back(tool);
}

void Tool::removeSubTool(const ToolPtr& tool)
{
    for (auto it=m_subTools.begin(); it!=m_subTools.end(); it++)
    {
        if (tool == *it)
        {
            m_subTools.erase(it);
            return;
        }
    }

    BMM_DEBUG() << "Tried to remove sub tool that does not exist!\n";
    throw std::exception();
}

bool Tool::onEvent(const Event& event)
{
    bool handled = false;

    // First dispatch event to active handler
    auto activeHandler = m_activeSubTool; // Keep a copy of the handler
    if (activeHandler)
    {
        handled = dispatchEventTo(event, activeHandler.get());
        if (activeHandler->isActive())
        {
            BMM_DEBUG() << "Sub tool still active\n";
            // The active handler is still active. 
            // Note: the handler might not have handled the event, but is still active
            return handled;
            
        }
        
        // The active handler is not active anymore, reset pointer
        m_activeSubTool = nullptr;
        BMM_DEBUG() << "Sub tool deactivated\n";
        if (handled)
        {
            // Even if not active, it might have handled the event. Return here if so.
            return true;
        }
    }

    // if (!isActive()) // TODO: what is we are active?
    // Event has not been handled yet. Dispatch the event to other handlers
    for (const auto& s : m_subTools)
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

        //BMM_DEBUG() << "Interaction handler handled event\n";
        if (s->isActive())
        {
            BMM_DEBUG() << "Interaction handler activated!\n";
            m_activeSubTool = s;
            return true;
        }
    }

    // No one handled it, handle it ourselfs.
    return EventHandler::onEvent(event);
}