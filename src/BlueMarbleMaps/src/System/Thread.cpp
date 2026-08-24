#include "BlueMarbleMaps/System/Thread.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include <iostream>
#include <cassert>

using namespace BlueMarble::System;

ThreadPool::ThreadPool(size_t numThreads, size_t maxQueueSize, QueuePolicy queuePolicy)
    : m_workers(numThreads)
    , m_tasks()
    , m_queueMutex()
    , m_maxQueueSize(maxQueueSize)
    , m_queuePolicy(queuePolicy)
    , m_condition()
    , m_stop(true)
{
    // We store the thread id such that we can verify that all calls to
    // this ThreadPool are made from the same thread.
    // The reason is a use case where "onDropped" can be hard to debug in the
    // scenario that shared resources are synched with a mutex outside of 
    // this thread pool. Forcing all calls to be made on the main thread,
    // guarantees that all calls to onDropped are as well.
    m_mainThreadId = std::this_thread::get_id();
}

ThreadPool::~ThreadPool()
{
    stop(true);
}

void ThreadPool::start(size_t numThreads, size_t maxQueueSize, QueuePolicy queuePolicy)
{
    assert(isValidThreadAccess());

    if (maxQueueSize == 0)
    {
        throw std::runtime_error("ThreadPool::start() max queuesize must be greater than 0");
    }

    if (m_stop)
    {
        m_stop = false;
    }
    else
    {
        throw std::runtime_error("ThreadPool::start() called while thread pool is already running");
    }

    m_workers.resize(numThreads);
    m_maxQueueSize = maxQueueSize;
    m_queuePolicy = queuePolicy;

    for (auto& worker : m_workers)
    {
        worker = std::thread([this] 
        {
            for (;;)
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_condition.wait(lock, [this] 
                { 
                    return m_stop || !m_tasks.empty(); 
                });

                if (m_stop && m_tasks.empty())
                {
                    BMM_DEBUG() << "Worker thread exiting\n";
                    return;
                }
                   
                // std::queue
                // Task task = std::move(m_tasks.front());
                // m_tasks.pop();

                // std::multiset
                auto it = m_tasks.begin();
                Task task = std::move(it->task);
                m_tasks.erase(it);

                lock.unlock();
                m_condition.notify_one();

                // Execute the task
                task.task();
            }
        });
    }

    BMM_DEBUG() << "Thread pool started " << m_workers.size() << " workers\n";
}

void ThreadPool::stop(bool dropQueuedTasks)
{
    assert(isValidThreadAccess());
    BMM_DEBUG() << "ThreadPool::stop()\n";
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
        if (dropQueuedTasks)
        {
            BMM_DEBUG() << "ThreadPool::stop() dropping queued tasks\n";
            while (!m_tasks.empty())
            {
                // std::queue
                // auto t = std::move(m_tasks.front());
                // m_tasks.pop();

                // std::multiset
                auto it = m_tasks.begin();
                Task task = std::move(it->task);
                m_tasks.erase(it);

                lock.unlock();
                task.onDropped();
                lock.lock();
            }
        }
    }
    m_condition.notify_all();
    for (std::thread& worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
    
}

void ThreadPool::enqueue(Task&& task)
{
    assert(isValidThreadAccess());
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        // don't allow enqueueing after stopping the pool
        if (m_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        if (m_tasks.size() >= m_maxQueueSize)
        {
            // BMM_DEBUG() << "ThreadPool queue is full (size: " << m_tasks.size() << "), applying queue policy\n";
            switch (m_queuePolicy)
            {
            case QueuePolicy::GrowWhenFull:
                // Just allow it to grow, no need to do anything here
                break;
            case QueuePolicy::BlockWhenFull:
                m_condition.wait(lock, [this](){ return m_tasks.size() < m_maxQueueSize || m_stop; });
                if (m_stop)                    throw std::runtime_error("enqueue on stopped ThreadPool");
                
                break;
            case QueuePolicy::DropWhenFull:
                // Just return and drop the task, no need to do anything here
                lock.unlock();
                task.onDropped();
                return;
            case QueuePolicy::ReplaceOldestWhenFull:
            {
                m_tasks.insert({
                    std::move(task),
                    m_nextTaskSequence++
                });
                auto lowest = std::prev(m_tasks.end());
                auto droppedTask = std::move(lowest->task);
                m_tasks.erase(lowest);

                // New task isn't more important than anything already queued.
                lock.unlock();
                droppedTask.onDropped();
                return;
                // // Remove the oldest task and add the new one
                // auto t = std::move(m_tasks.front());
                // m_tasks.pop();
                // m_tasks.emplace(std::move(task));

                // lock.unlock();

                // t.onDropped();

                // lock.lock();
                // break;
            }  
            default:
                throw std::runtime_error("Invalid queue policy");
            }
        }

        m_tasks.insert({
            std::move(task),
            m_nextTaskSequence++
        });
    }
    m_condition.notify_one();
}
