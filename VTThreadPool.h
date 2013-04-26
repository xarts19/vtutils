#pragma once

#include "VTThread.h"
#include "VTEvent.h"
#include "VTLock.h"
#include "VTFuture.h"

#include <memory>
#include <vector>
#include <functional>


namespace VT
{
    namespace d_
    {
        struct WorkData
        {
            WorkData(const std::function<void()> work,
                     const Promise& result)
               : work(work)
               , result(result)
            { }

            std::function<void()> work;
            Promise result;
        };


        class WorkerThread : public Thread
        {
        public:
            WorkerThread(VT::Event& free_notify);
            WorkerThread();  // no definition, used to prevent error in MSVC with make_shared
            ~WorkerThread();

            void do_work(const WorkData& work_data);
            void terminate();

            bool wait(int timeout_ms = -1);

            bool is_free();

        private:
            virtual void run();

            WorkData work_data_;
            bool terminated_;
            VT::Event working_;
            VT::Event free_;
            VT::Event& free_notify_;
        };
    }

    class ThreadPool : private Thread
    {
    public:
        ThreadPool(size_t thread_count = 8, size_t max_thread_count = 100);
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

        Future run(Thread* work);
        Future run(const std::function<void()>& work);

        bool wait_for_all(int timeout_ms = -1);

        static ThreadPool global;

    private:
        virtual void run();  // scheduler

        std::shared_ptr<d_::WorkerThread> free_thread();
        void add_thread();
        void adjust_thread_count(size_t count);

        size_t max_thread_count_;
        bool dead_;
        std::vector<std::shared_ptr<d_::WorkerThread>> threads_;
        std::vector<d_::WorkData> queue_;
        VT::Event has_queue_;
        VT::Event has_free_threads_;
        mutable VT::Lock lock_;
    };
}
