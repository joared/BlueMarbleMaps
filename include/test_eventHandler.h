#ifndef TEST_EVENTHANDLER
#define TEST_EVENTHANDLER

#include "EventHandler.h"

#define ASSERT_EQ_WARN(value, expected, errInfo)                            \
    if (value != expected)                                                  \
    {                                                                       \
        std::cout << errInfo << ": " << value << " != "<< expected << "\n"; \
    }                                                                       \

#define ASSERT_NEQ_WARN(value, expected, errInfo)                           \
    if (value == expected)                                                  \
    {                                                                       \
        std::cout << errInfo << ": " << value << " == "<< expected << "\n"; \
    }  

namespace BlueMarble
{
    class TestEventHandler : public EventHandler
    {
        public:
            bool OnDragBegin(const DragBeginEvent& event) override final
            {
                m_dragBeginTimeStamp = event.timeStampMs;
                return false;
            }

            bool OnDrag(const DragEvent& event) override final
            {
                ASSERT_NEQ_WARN(event.timeStampMs, m_dragBeginTimeStamp, "OnDrag timestamp test failed")
                return false;
            }

            bool OnDragEnd(const DragEndEvent& /*event*/) override final
            {
                // std::string errInfo = "OnDragEnd test failed ";
                // ASSERT_EQ_WARN(event.pos.x, event.lastPos.x, errInfo + "'x'")
                // ASSERT_EQ_WARN(event.pos.y, event.lastPos.y, errInfo + "'y'")

                return false;
            }
        private:
            int m_dragBeginTimeStamp;
    };

}
#endif /* TEST_EVENTHANDLER */
