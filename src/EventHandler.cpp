#include "EventHandler.h"

namespace BlueMarble
{
    EventHandler::EventHandler()
    {
    }

    EventDispatcher::EventDispatcher()
    {
    }

    void EventDispatcher::addSubscriber(EventHandler* eventHandler)
    {
        m_eventHandlers.push_back(eventHandler);
    }

    void EventDispatcher::dispatchEvent(Event &event, int timeStampMs)
    {
        event.timeStampMs = timeStampMs; // TODO: should not be here??
        for (auto eventHandler : m_eventHandlers)
        {
            if (eventHandler->handleEvent(event))
                return;
        }
    }

    EventManager::EventManager()
    {
    }

    void EventManager::dispatchEvent(Event &event, int timeStampMs)
    {
        EventDispatcher::dispatchEvent(event, timeStampMs);
        m_eventDispatched = true;
    }

} // namespace BlueMarble