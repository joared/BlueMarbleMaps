#include "BlueMarbleMaps/Event/EventHandler.h"
#include "BlueMarbleMaps/Event/PointerEvent.h"
#include "BlueMarbleMaps/Event/KeyEvent.h"

#include <iostream>

namespace BlueMarble
{
    EventHandler::EventHandler()
        : m_eventFilter(nullptr)
        , m_handleEventRecursionGuard(false)
    {
    }


    void EventHandler::installEventFilter(EventHandler *eventFilter)
    {
        assert(eventFilter != this);
        if (eventFilter != nullptr)
        {
            // Not allowed to have two EventHandlers to have eachother as EventFilter
            assert(eventFilter->m_eventFilter != this);
        }
        m_eventFilter = eventFilter;

    }

    bool EventHandler::onEventFilter(EventHandler* target, const Event& event) 
    {
        // Default implementation does nothing. If you want to dispatch
        // the event to an event callback, override this and call OnEvent(event)
        return false;
    }

    bool EventHandler::onEvent(const Event& event) 
    {
        return event.dispatch(this);
    }


    bool EventHandler::handleEvent(EventHandler* target, const Event& event)
    {
        if (m_handleEventRecursionGuard)
        {
            throw std::runtime_error("EventHandler::handleEvent called withing handle event!!!");
        }
        m_handleEventRecursionGuard = true;
        
        bool retVal = false;
        if (m_eventFilter && m_eventFilter->handleEvent(this, event))
        {
            // An installed event filter has handled the event
            retVal = true;
        }
        else if (target != nullptr)
        {
            // We are an event filter for "target"
            retVal = onEventFilter(target, event);
        }
        else
        {
            // "Standard": we have received an event
            retVal = onEvent(event);
        }
        
        m_handleEventRecursionGuard = false;

        return retVal;
    }


    EventDispatcher::EventDispatcher()
    {
    }

    void EventDispatcher::addSubscriber(EventHandler* eventHandler)
    {
        m_eventHandlers.push_back(eventHandler);
    }

    bool EventDispatcher::dispatchEvent(Event &event, int timeStampMs)
    {
        event.timeStampMs = timeStampMs; // TODO: should not be here??
        for (auto eventHandler : m_eventHandlers)
        {
            if (eventHandler->handleEvent(nullptr, event))
                return true;
        }

        return false;
    }

} // namespace BlueMarble