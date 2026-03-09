#include "BlueMarbleMaps/System/Thread.h"
#include "BlueMarbleMaps/Logging/Logging.h"

#include <iostream>

using namespace BlueMarble::System;

ThreadPool::ThreadPool()
    : m_workers(0)
    , m_tasks()
    , m_queueMutex()
    , m_maxQueueSize(0)
    , m_queuePolicy(QueuePolicy::GrowWhenFull)
    , m_condition()
    , m_stop(true)
{
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::start(size_t numThreads, size_t maxQueueSize, QueuePolicy queuePolicy)
{
    if (maxQueueSize <= 0)
    {
        throw std::runtime_error("ThreadPool::start() max queuesize must be greater than 0");
    }

    if (m_stop)
    {
        m_stop = false;
    }
    else
    {
        throw std::runtime_error("ThreadPool is already running");
    }

    m_workers.resize(numThreads);
    m_maxQueueSize = maxQueueSize;
    m_queuePolicy = queuePolicy;

    for (auto& worker : m_workers)
    {
        worker = std::thread([this] 
        {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_queueMutex);
                    m_condition.wait(lock, [this] 
                    { 
                        return m_stop || !m_tasks.empty(); 
                    });

                    if (m_stop && m_tasks.empty())
                        return;

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }
                task();
            }
            std::cout << "Worker thread exiting\n";
        });
    }
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (std::thread &worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(std::function<void()>&& task)
{
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
                return;
            case QueuePolicy::ReplaceOldestWhenFull:   
                // Remove the oldest task and add the new one
                m_tasks.pop();
                break;
            
            default:
                throw std::runtime_error("Invalid queue policy");
                break;
            }
        }

        m_tasks.emplace(std::move(task));
    }
    m_condition.notify_one();
}