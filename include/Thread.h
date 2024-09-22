#ifndef BLUEMARBLE_THREAD
#define BLUEMARBLE_THREAD

#include <thread>
#include <functional>

namespace BlueMarble
{

    class ThreadTerminationHandler
    {
        public:
            virtual void onThreadDone();
    };

    class Thread
    {
        public:
            Thread(const std::function<void>& job, const std::function<void>& terminationHandler);

        private:
            std::thread m_thread;
            const std::function<void>& m_job;
            const std::function<void>& m_terminationHandler;
    };

}

#endif /* BLUEMARBLE_THREAD */
