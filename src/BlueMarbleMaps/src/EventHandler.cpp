#include "EventHandler.h"
#include "PointerEvent.h"
#include "KeyEvent.h"

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

    bool EventDispatcher::dispatchEvent(Event &event, int timeStampMs)
    {
        event.timeStampMs = timeStampMs; // TODO: should not be here??
        for (auto eventHandler : m_eventHandlers)
        {
            if (eventHandler->handleEvent(event))
                return true;
        }

        return false;
    }

    EventManager::EventManager()
    {
    }

    bool EventManager::mouseDown(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        MouseDownEvent event;
        event.pos = ScreenPos{x,y};
        event.modificationKey = modKeys;
        event.mouseButton = button;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::mouseMove(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        MouseMoveEvent event;
        event.pos = ScreenPos{x,y};
        event.modificationKey = modKeys;
        event.mouseButton = button;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::mouseUp(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        MouseUpEvent event;
        event.pos = ScreenPos{x,y};
        event.modificationKey = modKeys;
        event.mouseButton = button;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::mouseWheel(int delta, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        MouseWheelEvent event;
        event.pos = ScreenPos{x,y};
        event.modificationKey = modKeys;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::keyUp(int key, ModificationKey modKeys, int64_t timeStampMs)
    {
        KeyUpEvent event;
        event.modificationKey = modKeys;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::keyDown(int key, ModificationKey modKeys, int64_t timeStampMs)
    {
        KeyDownEvent event;
        event.modificationKey = modKeys;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::resize(int width, int height, int64_t timeStampMs)
    {
        ResizeEvent event;
        event.width = width;
        event.height = height;
        return dispatchEvent(event, timeStampMs);
    }

    bool EventManager::dispatchEvent(Event &event, int timeStampMs)
    {
        m_eventDispatched = true;
        return EventDispatcher::dispatchEvent(event, timeStampMs);
    }

} // namespace BlueMarble