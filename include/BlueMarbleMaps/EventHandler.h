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
            virtual MouseButton getMouseButton() { throw std::exception(); }; // Window specific
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
