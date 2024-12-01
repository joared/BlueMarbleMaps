#ifndef EVENTHANDLER
#define EVENTHANDLER

#include "Event.h"
#include "Buttons.h"
#include "PointerEvent.h"
#include <vector>
#include <map>

namespace BlueMarble
{

    class EventHandler : public EventCallbacks
    {
        public:
            EventHandler();
            virtual bool handleEvent(const Event& event)
            {
                return event.execute(this);
            }
    };

    class EventDispatcher
    {
        public:
            EventDispatcher();
            void addSubscriber(EventHandler* eventHandler);
            bool dispatchEvent(Event& event, int timeStampMs);
        private:
            std::vector<EventHandler*> m_eventHandlers;
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
            virtual bool captureEvents() = 0;

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
            bool resize(int width, int height, int64_t timeStamp=-1);

            bool dispatchEvent(Event& event, int timeStampMs);
        protected:
            bool m_eventDispatched;

            virtual void getMousePos(ScreenPos& pos) const { }; // Window specific
            virtual ModificationKey getModificationKeyMask() const { }; // Window specific
            virtual MouseButton getMouseButton() { }; // Window specific
            virtual int getWheelDelta() {};
            virtual void captureKeyEvents() {};// Window specific
        
            void reset();
            bool mouseButtonChanged(MouseButton lastDown, MouseButton current);
            bool mousePosChanged(const ScreenPos& lastPos, const ScreenPos& currPos);
            void handlePosChanged(MouseButton lastDown, const ScreenPos& currPos);
            void handleMouseButtonChanged(MouseButton curr, const ScreenPos& currPos);
            bool detectDrag(const ScreenPos& startPos, const ScreenPos& currPos, int thresh);
            void captureMouseEvents();
            
            
            // Key events
            void handleKey(bool keyDownState, KeyButton key);

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
