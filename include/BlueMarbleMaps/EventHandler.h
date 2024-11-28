#ifndef EVENTHANDLER
#define EVENTHANDLER

#include "Event.h"
#include "Buttons.h"
#include <vector>

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
    };



} // namespace name

#endif /* EVENTHANDLER */
