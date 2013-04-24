#include "VTThreadPool.h"

#include "VTTimer.h"

#include <algorithm>

#include <assert.h>

VT::d_::WorkerThread::WorkerThread(VT::Event& free_notify)
    : work_()
    , terminated_(false)
    , free_notify_(free_notify)
{
    free_.signal();
    free_notify_.signal();
    start();
}


VT::d_::WorkerThread::~WorkerThread()
{
    terminate();
    join();
}


void VT::d_::WorkerThread::run()
{
    for (;;)
    {
        working_.wait();

        if (terminated_)
            return;

        assert(work_);
        work_();
        work_ = nullptr;
        working_.reset();
        free_.signal();
        free_notify_.signal();
    }
}


void VT::d_::WorkerThread::do_work(const std::function<void()>& work)
{
    work_ = work;
    free_.reset();
    working_.signal();
}


void VT::d_::WorkerThread::terminate()
{
    terminated_ = true;
    working_.signal();
}


bool VT::d_::WorkerThread::wait(int timeout_ms)
{
    return free_.wait(timeout_ms);
}


bool VT::d_::WorkerThread::is_free()
{
    return free_.is_signaled();
}


VT::ThreadPool::ThreadPool(size_t thread_count, size_t max_thread_count)
    : max_thread_count_(max_thread_count)
    , dead_(false)
{
    assert(thread_count >= 0);
    assert(max_thread_count >= 1);
    
    set_thread_count(thread_count);

    start();
}


VT::ThreadPool::~ThreadPool()
{
    wait_for_all();  // wait for work to finish

    {
        Locker l(lock_);
        dead_ = true;
        has_queue_.signal();         //  to force scheduler to exit the loop
    }

    join();  // join() (run() method) needs to take the lock

    // destructors of worker threads will take care of joining existing threads
}


size_t VT::ThreadPool::thread_count() const
{
    return threads_.size();
}


size_t VT::ThreadPool::max_thread_count() const
{
    return max_thread_count_;
}


void VT::ThreadPool::set_thread_count(size_t count)
{
    assert(count >= 0);
    
    Locker l(lock_);

    // Create new threads
    while (count > threads_.size())
    {
        add_thread();
    }

    // Trim existing threads
    while (count < threads_.size())
    {
        threads_.pop_back();  // destructor will call terminate() and join()
    }
}


void VT::ThreadPool::set_max_thread_count(size_t count)
{
    assert(count >= 1);
    
    max_thread_count_ = count;
    if (max_thread_count_ < threads_.size())
        set_thread_count(max_thread_count_);
}


void VT::ThreadPool::run(Thread* work)
{
    run(std::bind(std::mem_fn(&Thread::run), work));
}


void VT::ThreadPool::run(const std::function<void()>& work)
{
    Locker l(lock_);

    // do we have free threads?
    auto thread = free_thread();
    if (thread)
    {
        thread->do_work(work);
    }
    else if (threads_.size() < max_thread_count_)  // we still have free slots
    {
        add_thread();
        threads_.back()->do_work(work);
    }
    else  // queue that bitch
    {
        queue_.push_back(work);
        has_queue_.signal();
    }
}


bool VT::ThreadPool::wait_for_all(int timeout_ms)
{
    Timer timer;

    for (;;)
    {
        std::shared_ptr<d_::WorkerThread> thread;
        bool queue_empty = false;

        {
            Locker l(lock_);

            queue_empty = queue_.empty();

            for (auto it = threads_.begin(); it != threads_.end(); ++it)
            {
                if (!(*it)->is_free())
                {
                    thread = *it;
                    break;
                }
            }
        }

        if (!thread && queue_empty)
            return true;

        int time_left = -1;
        if (timeout_ms != -1)
        {
            time_left = static_cast<int>(timeout_ms - timer.time_elapsed_s() * 1000);
            if (time_left < 0)
                return false;
        }

        if (thread)
        {
            if (!thread->wait(time_left))
                return false;
        }
    }
}


void VT::ThreadPool::run()
{
    for (;;)
    {
        has_queue_.wait();
        has_free_threads_.reset();

        std::shared_ptr<d_::WorkerThread> thread;

        {
            Locker l(lock_);

            if (dead_)  // to exit when destructor sets dead_ and signals has_queue_
                break;

            thread = free_thread();

            if (!thread && threads_.size() < max_thread_count_)  // we still have free slots
            {
                add_thread();
                thread = threads_.back();
            }

            if (thread)
            {
                if (queue_.size() > 0)
                {
                    auto work = queue_.back();
                    queue_.pop_back();
                    thread->do_work(work);

                    if (queue_.empty())
                        has_queue_.reset();
                }
            }
        }

        if (!thread)
        {
            has_free_threads_.wait();
        }
    }
}


std::shared_ptr<VT::d_::WorkerThread> VT::ThreadPool::free_thread()
{
    auto it = std::find_if(threads_.begin(), threads_.end(), std::mem_fn(&d_::WorkerThread::is_free));
    if (it != threads_.end())
        return *it;
    else
        return std::shared_ptr<d_::WorkerThread>();
}


void VT::ThreadPool::add_thread()
{
    threads_.push_back(std::make_shared<d_::WorkerThread>(has_free_threads_));
}
