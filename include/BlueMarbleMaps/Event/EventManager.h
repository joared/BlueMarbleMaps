#ifndef BLUEMARBLE_EVENTMANAGER
#define BLUEMARBLE_EVENTMANAGER

#include "BlueMarbleMaps/Event/EventHandler.h"
#include "BlueMarbleMaps/Event/Event.h"
#include "BlueMarbleMaps/Core/Buttons.h"
#include "BlueMarbleMaps/Event/PointerEvent.h"
#include "BlueMarbleMaps/Event/KeyEvent.h"
#include "BlueMarbleMaps/Event/TimerEvent.h"

#include <vector>
#include <map>
#include <stdexcept>

namespace BlueMarble
{
    class EventConfiguration
    {
        public:
            // Double click
            int doubleClickTimeoutMs = 500;
            int doubleClickThresh = 5;
            // Drag
            int dragThresh = 5;
    };

    class EventManager 
        : public EventDispatcher // TODO:  shold be hidden when using mapcontrol
    {
        public:
            EventManager();

            // Timer event. TODO: Add support for multiple timers using id
            bool timer(int64_t id, int64_t timeStamp);

            void setTimer(EventHandler* handler, int64_t interval);
            void killTimer(EventHandler* handler);
            virtual int64_t setTimer(int64_t interval) = 0; // Window specific. Returns timer id.
            virtual bool killTimer(int64_t id) = 0;

            // Mouse events
            bool mouseDown(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStamp);
            bool mouseMove(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStamp);
            bool mouseUp(MouseButton button, int x, int y, ModificationKey modKeys, int64_t timeStamp);
            bool mouseWheel(int delta, int x, int y, ModificationKey modKeys, int64_t timeStamp);

            virtual void getMousePos(ScreenPos& pos) const { throw std::exception(); }; // Window specific
            virtual ModificationKey getModificationKeyMask() const { throw std::exception(); }; // Window specific
            virtual MouseButton getMouseButton() const { throw std::exception(); }; // Window specific

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

            // Timer event
            TimerEvent m_previousTimerEvent;
            std::map<int64_t, EventHandler*> m_timerEventHandlers;

            // Mouse events
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
}

#endif /* BLUEMARBLE_EVENTMANAGER */
