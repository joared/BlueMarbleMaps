#ifndef EVENTHANDLER
#define EVENTHANDLER

#include "BlueMarbleMaps/Event/Event.h"
#include "BlueMarbleMaps/Core/Buttons.h"
#include "BlueMarbleMaps/Event/PointerEvent.h"
#include <vector>
#include <map>
#include <stdexcept>

namespace BlueMarble
{
    // Forward declaration
    class EventHandler;

    class EventDispatcher
    {
        public:
            EventDispatcher();
            void addSubscriber(EventHandler* eventHandler);
            bool dispatchEvent(Event& event, int timeStampMs);
        private:
            std::vector<EventHandler*> m_eventHandlers;
    };

    class EventHandler : protected EventCallbacks
    {
        public:
            EventHandler();
            virtual ~EventHandler() = default;

            void installEventFilter(EventHandler* eventFilter)
            {
                assert(eventFilter != this);
                if (eventFilter != nullptr)
                {
                    // Not allowed to have two EventHandlers to have eachother as EventFilter
                    assert(eventFilter->m_eventFilter != this);
                }
                m_eventFilter = eventFilter;

            }
            friend bool EventDispatcher::dispatchEvent(Event&, int);
        protected:
            
            virtual bool OnEventFilter(EventHandler* target, const Event& event) 
            {
                // Default implementation does nothing. If you want to dispatch
                // the event to an event callback, override this and call OnEvent(event)
                return false;
            }

            virtual bool OnEvent(const Event& event) 
            {
                return event.dispatch(this);
            }

        private:

            bool handleEventOld(const Event& event)
            {
                if (m_handleEventRecursionGuard)
                {
                    throw std::runtime_error("EventHandler::handleEvent called withing handle event!!!");
                }
                
                
                if (m_eventFilter && m_eventFilter->handleEventOld(event))
                    return true;

                m_handleEventRecursionGuard = true;
                bool retVal = OnEvent(event);
                m_handleEventRecursionGuard = false;

                return retVal;
            }

            bool handleEvent(EventHandler* target, const Event& event)
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
                    retVal = OnEventFilter(target, event);
                }
                else
                {
                    // "Standard": we have received an event
                    retVal = OnEvent(event);
                }
                
                m_handleEventRecursionGuard = false;

                return retVal;
            }
            
            EventHandler* m_eventFilter;
            bool          m_handleEventRecursionGuard;
    };

    class EventConfiguration
    {
        public:
            // Double click
            int doubleClickTimeoutMs = 500;
            int doubleClickThresh = 5;
            // Drag
            int dragThresh = 5;
    };

    class EventManager : public EventDispatcher
    {
        public:
            EventManager();

            // Mouse events
            bool mouseDown(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStamp);
            bool mouseMove(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStamp);
            bool mouseUp(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStamp);
            bool mouseWheel(int delta, int x, int y, ModificationKey modKeys, int64_t timeStamp);

            // Key events
            bool keyUp(int key, ModificationKey modKeys, int64_t timeStamp);
            bool keyDown(int key, ModificationKey modKeys, int64_t timeStamp);

            // Touch events (TODO)
            void touchBegin() {}
            void touchDown(uint64_t id, double x, double y, double radius, int64_t timeStamp) {}
            void touchMove(uint64_t id, double x, double y, double radius, int64_t timeStamp) {}
            void touchUp(uint64_t id, double x, double y, double radius, int64_t timeStamp) {}
            void touchEnd() {}

            // Other window related events
            virtual bool resize(int width, int height, int64_t timeStamp);

            // Use this for polling events. NOTE: need to implement window specific (virtual) getters below
            bool captureEvents();
        
            virtual void getMousePos(ScreenPos& pos) const { throw std::exception(); }; // Window specific
            virtual ModificationKey getModificationKeyMask() const { throw std::exception(); }; // Window specific
            virtual MouseButton getMouseButton() const { throw std::exception(); }; // Window specific
        protected:
            virtual int getWheelDelta() { throw std::exception(); }; // Window
            virtual bool getResize(int& width, int& height) { throw std::exception(); } // Window specific
            virtual bool captureKeyEvents() { throw std::exception(); };// Window specific
            // Key events
            void handleKey(bool keyDownState, KeyButton key);
        private:
            void reset();
            bool captureMouseEvents();
            bool mouseButtonChanged(MouseButton lastDown, MouseButton current);
            bool mousePosChanged(const ScreenPos& lastPos, const ScreenPos& currPos);
            void handlePosChanged(MouseButton lastDown, const ScreenPos& currPos, ModificationKey modKeys);
            void handleMouseButtonChanged(MouseButton curr, const ScreenPos& currPos, ModificationKey modKeys);
            bool detectDrag(const ScreenPos& startPos, const ScreenPos& currPos, int thresh);
            

            EventConfiguration m_configration;

            // For mouse events
            int m_timeStampMs;
            int m_downTimeStampMs;
            ScreenPos m_downPos;
            ScreenPos m_lastPos;
            MouseButton m_lastDown;
            MouseDownEvent m_previousMouseDownEvent;
            bool m_isDragging;

            // For key events
            std::map<KeyButton, bool> m_keyButtonDownMap;
    };



} // namespace name

#endif /* EVENTHANDLER */
