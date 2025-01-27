#ifndef KEYEVENT
#define KEYEVENT

#include "Event/Event.h"
#include "Event/PointerEvent.h"
#include "Core/Buttons.h"

namespace BlueMarble
{
    class KeyEvent : public PointerEvent
    {
        public:
            KeyButton keyButton;
    };

    class KeyDownEvent : public KeyEvent
    {
        public:
            DEFINE_EVENT(KeyDown)
    };

    class KeyUpEvent : public KeyEvent
    {
        public:
            DEFINE_EVENT(KeyUp)
    };

} // namespace BlueMarble


#endif /* KEYEVENT */
