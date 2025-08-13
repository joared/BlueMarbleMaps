#ifndef KEYEVENT
#define KEYEVENT

#include "BlueMarbleMaps/Event/Event.h"
#include "BlueMarbleMaps/Event/PointerEvent.h"
#include "BlueMarbleMaps/Core/Buttons.h"

namespace BlueMarble
{
    class KeyEvent : public PointerEvent
    {
        public:
            KeyButton keyButton;
            int keyCode;
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
