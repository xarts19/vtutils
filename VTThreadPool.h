#pragma once

#include "VTEvent.hpp"

#include <functional>
#include <future>


namespace VT
{
    class ThreadPool
    {
    public:
        ThreadPool(size_t thread_count = 1, size_t max_thread_count = 10);
        ~ThreadPool();

        // number of threads can grow when needed
        // thread count can't grow larger then max_thread_count
        //
        // if setting count or max_count reduces current thread number,
        // this operation may cause a wait for threads to finish

        size_t thread_count() const;
        size_t max_thread_count() const;
        void set_thread_count(size_t count);
        void set_max_thread_count(size_t count);

        std::future<void> run(const std::function<void()>& work);

        bool wait_for_all(int timeout_ms = -1);

        //static ThreadPool global;

    private:
        struct Impl;
        Impl* d;
    };

    class ThreadPoolSingleton
    {
    public:
        static ThreadPool& instance()
        {
            static ThreadPool tp;
            return tp;
        }
    };
}
