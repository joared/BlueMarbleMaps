#ifndef BLUEMARBLE_THREAD
#define BLUEMARBLE_THREAD

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>

namespace BlueMarble::System
{
    
class ThreadPool
{
public:
    enum class QueuePolicy
    {
        GrowWhenFull,
        BlockWhenFull,
        DropWhenFull,
        ReplaceOldestWhenFull
    };

    ThreadPool();
    ~ThreadPool();
    void start(size_t numThreads = std::thread::hardware_concurrency(), size_t maxQueueSize = 50, QueuePolicy queuePolicy = QueuePolicy::GrowWhenFull);
    void stop();
    bool isRunning() const { return !m_stop; }
    // template<class F, class... Args>
    // auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    void enqueue(std::function<void()>&& task);
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    size_t m_maxQueueSize;
    std::mutex m_queueMutex;
    QueuePolicy m_queuePolicy;
    std::condition_variable m_condition;
    std::atomic<bool> m_stop;
};

// template <class F, class... Args>
// inline auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
// {
//     using return_type = typename std::result_of<F(Args...)>::type;

//     auto task = std::make_shared<std::packaged_task<return_type()>>(
//         std::bind(std::forward<F>(f), std::forward<Args>(args)...)
//     );

//     std::future<return_type> res = task->get_future();
//     {
//         std::unique_lock<std::mutex> lock(m_queueMutex);

//         // don't allow enqueueing after stopping the pool
//         if (m_stop)
//             throw std::runtime_error("enqueue on stopped ThreadPool");

//         m_tasks.emplace([task]() { (*task)(); });
//     }
//     m_condition.notify_one();

//     return res;
// }

}

#endif /* BLUEMARBLE_THREAD */
