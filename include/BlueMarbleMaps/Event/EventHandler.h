#ifndef EVENTHANDLER
#define EVENTHANDLER

#include "BlueMarbleMaps/Event/Event.h"
#include <vector>
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
            void removeSubscriber(EventHandler* eventHandler);
            bool dispatchEvent(Event& event, int timeStampMs);
            bool dispatchEventTo(const Event& event, EventHandler* eventHandler);
        private:
            std::vector<EventHandler*> m_eventHandlers;
    };

    class EventHandler 
        : private EventCallbacks // Callbacks are privately inherited to prevent bypassing handleEvent
    {
        public:
            EventHandler();
            virtual ~EventHandler() = default;
            void installEventFilter(EventHandler* eventFilter);

            // Only an event dispatcher is allowed to dispatch events (by calling handleEvent)
            //friend bool EventDispatcher::dispatchEvent(Event&, int);
            friend bool EventDispatcher::dispatchEventTo(const Event&, EventHandler*);
        protected:
            // Default behaviour return false
            virtual bool onEventFilter(const Event& event, EventHandler* target=nullptr);
            // Default behaviour delegate to corresponding EventCallbacks method
            virtual bool onEvent(const Event& event);

        private:
            bool handleEvent(const Event& event, EventHandler* target=nullptr);
            
            EventHandler* m_eventFilter;
            bool          m_handleEventRecursionGuard;
    };

} // namespace name

#endif /* EVENTHANDLER */
