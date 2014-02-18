#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace VT
{
    template <typename T>
    class ThreadSafeQueue
    {
    public:
        void push(T&& item)
        {
            std::unique_lock<std::mutex> mlock(mutex_);
            queue_.push(std::forward<T>(item));
            mlock.unlock();     // unlock before notificiation to minimize mutex contention
            cond_.notify_one(); // notify one waiting thread
        }

        T pop()
        {
            std::unique_lock<std::mutex> mlock(mutex_);

            while (queue_.empty()) // check condition to be safe against spurious wakes
            {
                cond_.wait(mlock); // release lock and go join the waiting thread queue
            }

            auto item = std::move(queue_.front());
            queue_.pop();
            return item;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> mlock(mutex_);
            return queue_.empty();
        }

    private:
        std::queue<T> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cond_;
    };
}

