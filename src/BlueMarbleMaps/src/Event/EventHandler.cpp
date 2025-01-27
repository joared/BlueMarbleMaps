#include "Event/EventHandler.h"
#include "Event/PointerEvent.h"
#include "Event/KeyEvent.h"

#include <iostream>

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
        : m_timeStampMs(0)
        , m_downTimeStampMs(0)
    {
        reset();
    }

    bool EventManager::captureEvents()
    {
        m_timeStampMs = getTimeStampMs();
        bool handled = captureMouseEvents();
        handled = captureKeyEvents() || handled;
        int w, h;
        if (getResize(w, h))
        {
            handled = resize(w, h, m_timeStampMs) || handled;
        }

        return handled;
    }

    bool EventManager::mouseDown(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        m_timeStampMs = timeStampMs;
        handleMouseButtonChanged(button, ScreenPos{x,y}, modKeys);
        // MouseDownEvent event;
        // event.pos = ScreenPos{x,y};
        // event.modificationKey = modKeys;
        // event.mouseButton = button;
        // return dispatchEvent(event, timeStampMs);
        return true;
    }

    bool EventManager::mouseMove(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        m_timeStampMs = timeStampMs;
        handlePosChanged(button, ScreenPos{x,y}, modKeys);
        // MouseMoveEvent event;
        // event.pos = ScreenPos{x,y};
        // event.modificationKey = modKeys;
        // event.mouseButton = button;
        // return dispatchEvent(event, timeStampMs);
        return true;
    }

    bool EventManager::mouseUp(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        m_timeStampMs = timeStampMs;
        handleMouseButtonChanged(MouseButton::MouseButtonNone, ScreenPos{x,y}, modKeys);
        // MouseUpEvent event;
        // event.pos = ScreenPos{x,y};
        // event.modificationKey = modKeys;
        // event.mouseButton = button;
        // return dispatchEvent(event, timeStampMs);
        return true;
    }

    bool EventManager::mouseWheel(int delta, int x, int y, ModificationKey modKeys, int64_t timeStampMs)
    {
        MouseWheelEvent event;
        event.delta = delta;
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

    void EventManager::reset()
    {
        m_lastDown = MouseButtonNone;
        m_isDragging = false;
        m_downPos = ScreenPos();
        m_lastPos = ScreenPos();

        m_keyButtonDownMap[KeyButton::ArrowDown] = false;
        m_keyButtonDownMap[KeyButton::ArrowUp] = false;
        m_keyButtonDownMap[KeyButton::ArrowLeft] = false;
        m_keyButtonDownMap[KeyButton::ArrowRight] = false;
        m_keyButtonDownMap[KeyButton::Enter] = false;
        m_keyButtonDownMap[KeyButton::Space] = false;
        m_keyButtonDownMap[KeyButton::BackSpace] = false;
        m_keyButtonDownMap[KeyButton::One] = false;
        m_keyButtonDownMap[KeyButton::Two] = false;
        m_keyButtonDownMap[KeyButton::Three] = false;
        m_keyButtonDownMap[KeyButton::Four] = false;
        m_keyButtonDownMap[KeyButton::Five] = false;
        m_keyButtonDownMap[KeyButton::Six] = false;
        m_keyButtonDownMap[KeyButton::Seven] = false;
        m_keyButtonDownMap[KeyButton::Eight] = false;
        m_keyButtonDownMap[KeyButton::Nine] = false;
        m_keyButtonDownMap[KeyButton::Add] = false;
        m_keyButtonDownMap[KeyButton::Subtract] = false;
    }

    bool EventManager::mouseButtonChanged(MouseButton lastDown, MouseButton current)
    {
        return lastDown != current;
    }

    bool EventManager::mousePosChanged(const ScreenPos &lastPos, const ScreenPos &currPos)
    {
        return !(currPos.x == -1 || currPos.y == -1) && (lastPos.x != currPos.x || lastPos.y != currPos.y);
    }

    void EventManager::handlePosChanged(MouseButton lastDown, const ScreenPos& currPos, ModificationKey modKeys)
    {
        if (!m_isDragging)
        {
            if (lastDown != MouseButtonNone && detectDrag(m_downPos, currPos, m_configration.dragThresh))
            {
                // Drag begin, use only down pos
                DragBeginEvent beginEvent;
                beginEvent.startPos = m_downPos;
                beginEvent.lastPos = m_downPos;
                beginEvent.pos = m_downPos;
                beginEvent.modificationKey = modKeys;
                beginEvent.mouseButton = lastDown;
                dispatchEvent(beginEvent, m_previousMouseDownEvent.timeStampMs); // Use the timestamp of the mouse down event

                // Drag event directly after, using current pos
                DragEvent event;
                event.startPos = m_downPos;
                event.lastPos = m_downPos;
                event.pos = currPos;
                event.modificationKey = modKeys;
                event.mouseButton = m_lastDown;
                dispatchEvent(event, m_timeStampMs);

                m_isDragging = true;
            }
            else 
            {
                // Mouse move
                MouseMoveEvent event;
                event.pos = currPos;
                event.modificationKey = modKeys;
                event.mouseButton = lastDown;
                dispatchEvent(event, m_timeStampMs);
            }
        }
        else
        {
            // Drag
            DragEvent event;
            event.startPos = m_downPos;
            event.lastPos = m_lastPos;
            event.pos = currPos;
            event.modificationKey = modKeys;
            event.mouseButton = lastDown;     // We use the last down button
            dispatchEvent(event, m_timeStampMs);
        }
        m_lastPos = currPos;
    }

    void EventManager::handleMouseButtonChanged(MouseButton currButton, const ScreenPos& currPos, ModificationKey modKeys)
    {
        if (m_lastDown == MouseButtonNone)
        {
            m_downPos = currPos; // Store down position for drag detection
            m_lastDown = currButton; // Store last down button for mouse up

            if (m_previousMouseDownEvent.mouseButton == currButton                                                  // Same button (or any butt none?)
                && m_timeStampMs - m_previousMouseDownEvent.timeStampMs < m_configration.doubleClickTimeoutMs    // No timeout
                && !detectDrag(m_previousMouseDownEvent.pos, currPos, m_configration.doubleClickThresh))            // Within threshold
            {
                // Double click
                // TODO: Double click drag
                DoubleClickEvent clickEvent;
                clickEvent.pos = currPos;
                clickEvent.modificationKey = modKeys;
                clickEvent.mouseButton = currButton;
                dispatchEvent(clickEvent, m_timeStampMs);

                m_previousMouseDownEvent = MouseDownEvent(); // Reset such that no click event occurs in next mouse down
            }
            else
            {
                // Mouse down
                MouseDownEvent event;
                event.pos = currPos;
                event.modificationKey = modKeys;
                event.mouseButton = currButton;
                dispatchEvent(event, m_timeStampMs);

                m_previousMouseDownEvent = event; //TODO: Store the mouse down event for double click
            }
        }
        else if (currButton == MouseButtonNone)
        {
            if (!m_isDragging)
            {
                // Mouse up TOOD: should we dispatch mouse up?
                MouseUpEvent event;
                event.pos = currPos;
                event.modificationKey = modKeys;
                event.mouseButton = m_lastDown; // We use the last down button
                dispatchEvent(event, m_timeStampMs);

                // Also dispatch click if we have a previous mouse down event (not double click)
                if (m_previousMouseDownEvent.mouseButton != MouseButtonNone)
                {
                    ClickEvent clickEvent;
                    clickEvent.pos = currPos;
                    clickEvent.modificationKey = modKeys;
                    clickEvent.mouseButton = m_lastDown; // We use the last down button
                    dispatchEvent(clickEvent, m_timeStampMs);
                }
            }
            else
            {
                // Drag end
                DragEndEvent event;
                event.startPos = m_downPos;
                event.lastPos = m_lastPos;
                event.pos = currPos;
                event.modificationKey = modKeys;
                event.mouseButton = m_lastDown; // We use the last down button
                dispatchEvent(event, m_timeStampMs);

                m_isDragging = false; // Reset
            }
            m_lastDown = MouseButtonNone; // Reset
        }
        else 
        {
            // A mouse button was pressed while another was already down.
            // Ignore for now.
        }
    }

    bool EventManager::detectDrag(const ScreenPos &startPos, const ScreenPos &currPos, int thresh)
    {
        return abs(startPos.x - currPos.x) > thresh || abs(startPos.y - currPos.y) > thresh;
    }

    bool EventManager::captureMouseEvents()
    {
        MouseButton mouseButton = getMouseButton();
        bool mButtonChanged = mouseButtonChanged(m_lastDown, mouseButton);
        
        ScreenPos currScreenPos;
        getMousePos(currScreenPos);
        ModificationKey modKeys = getModificationKeyMask();
        bool posChanged = mousePosChanged(m_lastPos, currScreenPos);

        // We need to handle button changes first, since CImg
        // stores mouse positions of button press/release events.
        // This can cause erronous mouse position updates if position
        // changes are handled first.
        // bool first = m_lastDown != MouseButtonNone;
        if (mButtonChanged)
        {
            handleMouseButtonChanged(mouseButton, currScreenPos, modKeys);
        }

        // Handle move event first
        if (posChanged)
        {
            handlePosChanged(m_lastDown, currScreenPos, modKeys);
            m_lastPos = currScreenPos;
        }


        // Mouse wheel
        if (int wheelDelta = getWheelDelta())
        {
            // MouseWheelEvent event;
            // event.mouseButton = mouseButton;
            // event.pos = currScreenPos;
            // event.delta = wheelDelta;
            // dispatchEvent(event, m_timeStampMs);
            mouseWheel(wheelDelta, currScreenPos.x, currScreenPos.y, getModificationKeyMask(), m_timeStampMs);
        }

        return true; // TODO
    }

    

    void EventManager::handleKey(bool keyIsDown, KeyButton key)
    {
        if (keyIsDown == m_keyButtonDownMap[key])
            return;

        m_keyButtonDownMap[key] = keyIsDown;
        ScreenPos pos;
        getMousePos(pos);
        ModificationKey modKey = getModificationKeyMask();
        if (keyIsDown)
        {
            // Key down
            KeyDownEvent event;
            event.pos = pos;
            event.modificationKey = modKey;
            event.keyButton = key;
            dispatchEvent(event, m_timeStampMs);
        }
        else
        {
            // Key up
            KeyUpEvent event;
            event.pos = pos;
            event.modificationKey = modKey;
            event.keyButton = key;
            dispatchEvent(event, m_timeStampMs);
        }
    }

} // namespace BlueMarble