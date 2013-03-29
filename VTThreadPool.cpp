#include "VTThreadPool.h"

#include <algorithm>

#include <assert.h>

VT::d_::WorkerThread::WorkerThread(VT::Event& free_event)
    : work_()
    , terminated_(false)
    , free_event_(free_event)
{
    start();
}


VT::d_::WorkerThread::~WorkerThread()
{
    terminate();
    join();
}


void VT::d_::WorkerThread::run()
{
    while (!terminated_)
    {
        working_.wait();

        if (terminated_)
            return;

        assert(work_);
        work_();
        working_.reset();
        free_event_.signal();
    }
}


void VT::d_::WorkerThread::do_work(const std::function<void()>& work)
{
    work_ = work;
    working_.signal();
}


void VT::d_::WorkerThread::terminate()
{
    terminated_ = true;
    working_.signal();  // TODO: may be a bug here
                        // with interactions in run() method
                        // maybe we need to lock terminated_ variable
}


bool VT::d_::WorkerThread::wait(int timeout)
{
    return working_.wait(timeout);
}


bool VT::d_::WorkerThread::is_free()
{
    return (false == working_.wait(0));  // if we timeouted - it's free
}


VT::ThreadPool::ThreadPool(int thread_count, int max_thread_count)
    : max_thread_count_(max_thread_count)
    , dead_(false)
{
    set_thread_count(thread_count);

    start();
}


VT::ThreadPool::~ThreadPool()
{
    {
        Locker l(lock_);
        dead_ = true;
        has_queue_.signal();         //  to force scheduler to exit the loop
        has_free_threads_.signal();  //
    }

    join();  // join() (run() method) needs to take the lock

    // destructors of worker threads will take care of joining them
}


int VT::ThreadPool::thread_count() const
{
    return static_cast<int>(threads_.size());
}


int VT::ThreadPool::max_thread_count() const
{
    return max_thread_count_;
}


void VT::ThreadPool::set_thread_count(int count)
{
    // Create new threads
    while (count > threads_.size())
    {
        threads_.push_back(std::make_shared<d_::WorkerThread>(has_free_threads_));
    }

    // Trim existing threads
    while (count < threads_.size())
    {
        threads_.pop_back();  // destructor will call terminate() and join()
    }
}


void VT::ThreadPool::set_max_thread_count(int count)
{
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
        threads_.push_back(std::make_shared<d_::WorkerThread>(has_free_threads_));
        threads_.back()->do_work(work);
    }
    else  // queue that bitch
    {
        queue_.push_back(work);
        has_queue_.signal();
    }
}


bool VT::ThreadPool::wait_for_all(int timeout)
{
    for (auto it = threads_.begin(); it != threads_.end(); ++it)
    {
        if (false == (*it)->wait(timeout))
            return false;
    }

    return true;
}


void VT::ThreadPool::run()
{
    for (;;)
    {
        has_queue_.wait();

        std::shared_ptr<d_::WorkerThread> thread;

        {
            Locker l(lock_);

            if (dead_)  // to exit when destructor sets dead_ and signals has_queue_
                break;

            thread = free_thread();

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
            has_free_threads_.reset();
            has_free_threads_.wait();
        }

        Locker l(lock_);

        if (dead_)
            break;
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
