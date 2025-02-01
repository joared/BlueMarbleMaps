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
            bool dispatchEvent(Event& event, int timeStampMs);
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
            friend bool EventDispatcher::dispatchEvent(Event&, int);
        protected:
            // Default behaviour return false
            virtual bool onEventFilter(EventHandler* target, const Event& event);
            // Default behaviour delegate to corresponding EventCallbacks method
            virtual bool onEvent(const Event& event);

        private:
            bool handleEvent(EventHandler* target, const Event& event);
            
            EventHandler* m_eventFilter;
            bool          m_handleEventRecursionGuard;
    };

} // namespace name

#endif /* EVENTHANDLER */
