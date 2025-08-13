#ifndef BLUEMARBLE_TIMEREVENT
#define BLUEMARBLE_TIMEREVENT

#include "Event.h"

namespace BlueMarble
{
    class TimerEvent : public Event
	{
		public:
			DEFINE_EVENT(Timer);
            int64_t id;
            int64_t deltaMs;
	};
}

#endif /* BLUEMARBLE_TIMEREVENT */
