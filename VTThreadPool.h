#pragma once

#include "VTThread.h"
#include "VTEvent.h"
#include "VTCriticalSection.h"

#include <memory>
#include <vector>
#include <functional>


namespace VT
{
    namespace d_
    {
        class WorkerThread : public Thread
        {
        public:
            WorkerThread();  // no definition, used to prevent error in MSVC with make_shared
            explicit WorkerThread(VT::Event& free_event);
            ~WorkerThread();

            void do_work(const std::function<void()>& work);
            void terminate();
            bool wait(int timeout = -1);

            bool is_free();

        private:
            virtual void run();

            std::function<void()> work_;
            bool terminated_;
            VT::Event working_;
            VT::Event& free_event_;
        };
    }

    class ThreadPool : private Thread
    {
    public:
        ThreadPool(int thread_count = 8, int max_thread_count = 100);
        ~ThreadPool();

        // number of threads can grow when needed
        // thread count can't grow larger then max_thread_count
        //
        // if setting count or max_count reduces current thread number,
        // this operation may cause a wait for threads to finish

        int thread_count() const;
        int max_thread_count() const;
        void set_thread_count(int count);
        void set_max_thread_count(int count);

        void run(Thread* work);
        void run(const std::function<void()>& work);

        bool wait_for_all(int timeout = -1);

    private:
        virtual void run();  // scheduler

        std::shared_ptr<d_::WorkerThread> free_thread();

        int max_thread_count_;
        bool dead_;
        std::vector<std::shared_ptr<d_::WorkerThread>> threads_;
        std::vector<std::function<void()>> queue_;
        VT::Event has_queue_;
        VT::Event has_free_threads_;
        VT::CriticalSection lock_;
    };
}
