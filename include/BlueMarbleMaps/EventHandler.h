#ifndef EVENTHANDLER
#define EVENTHANDLER

#include "Event.h"
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
            void dispatchEvent(Event& event, int timeStampMs);
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
            void dispatchEvent(Event& event, int timeStampMs);
        protected:
            bool m_eventDispatched;
    };



} // namespace name




#endif /* EVENTHANDLER */
