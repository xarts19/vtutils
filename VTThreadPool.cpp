#include "VTThreadPool.h"

#include "VTTimer.h"
#include "VTUtil.h"

#include <algorithm>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>

#include <assert.h>

#define GLOBAL_THREAD_POOL_SIZE 1
#define GLOBAL_THREAD_POOL_MAX_SIZE 10

//VT::ThreadPool VT::ThreadPool::global(GLOBAL_THREAD_POOL_SIZE, GLOBAL_THREAD_POOL_MAX_SIZE);


namespace VT
{
    namespace d_
    {
        struct WorkData
        {
            WorkData(const std::function<void()> work,
                     std::promise<void>&& result)
               : work(work)
               , result(std::move(result))
            { }

            WorkData(WorkData&& other)
                : work(other.work)
                , result(std::move(other.result))
            { }

            WorkData& operator=(WorkData&& rhs)
            {
                work = rhs.work;
                result = std::move(rhs.result);
                return *this;
            }

            std::function<void()> work;
            std::promise<void> result;
        };

        class WorkerThread
        {
        public:
            WorkerThread(VT::Event& free_notify);
            WorkerThread();  // no definition, used to prevent error in MSVC with make_shared
            ~WorkerThread();

            void do_work(WorkData&& work_data);
            void terminate();

            bool wait(int timeout_ms = -1);

            bool is_free();

        private:
            void worker();

            WorkData work_data_;
            bool terminated_;
            VT::Event working_;
            VT::Event free_;
            VT::Event& free_notify_;
            std::thread thread_;
        };
    }
}


VT::d_::WorkerThread::WorkerThread(VT::Event& free_notify)
    : work_data_(nullptr, std::promise<void>())
    , terminated_(false)
    , free_notify_(free_notify)
    , thread_(std::bind(std::mem_fn(&WorkerThread::worker), this))
{
    free_.signal();
    free_notify_.signal();
}


VT::d_::WorkerThread::~WorkerThread()
{
    terminate();
    thread_.join();
}


void VT::d_::WorkerThread::worker()
{
    for (;;)
    {
        working_.wait();

        if (terminated_)
            return;

        assert(work_data_.work);

        try
        {
            work_data_.work();
            work_data_.result.set_value();
        }
        catch (...)
        {
            work_data_.result.set_exception(std::current_exception());
        }

        work_data_.work = nullptr;
        working_.reset();
        free_.signal();
        free_notify_.signal();
    }
}


void VT::d_::WorkerThread::do_work(WorkData&& work_data)
{
    work_data_ = std::move(work_data);
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


struct VT::ThreadPool::Impl
{
    explicit Impl(size_t max_thread_count)
        : max_thread_count_(max_thread_count)
        , dead_(false)
        , thread_(std::bind(std::mem_fn(&ThreadPool::Impl::scheduler_loop), this))
    {
        assert(max_thread_count >= 1);
    }

    ~Impl()
    {
        {
            std::lock_guard<std::mutex> l(lock_);
            dead_ = true;
            has_queue_.signal();         //  to force scheduler to exit the loop
        }
        // thread_.join() will take the lock, so we need to release it here
        thread_.join();
        // destructors of worker threads will take care of joining existing threads
    }

    std::shared_ptr<VT::d_::WorkerThread> free_thread()
    {
        auto it = std::find_if(threads_.begin(), threads_.end(), std::mem_fn(&d_::WorkerThread::is_free));
        if (it != threads_.end())
            return *it;
        else
            return std::shared_ptr<d_::WorkerThread>();
    }

    void add_thread()
    {
        threads_.push_back(std::make_shared<d_::WorkerThread>(has_free_threads_));
    }

    void adjust_thread_count(size_t count)
    {
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

    void scheduler_loop()
    {
        for (;;)
        {
            has_queue_.wait();
            has_free_threads_.reset();

            std::shared_ptr<d_::WorkerThread> thread;

            {
                std::lock_guard<std::mutex> l(lock_);

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
                        auto work_data = std::move(queue_.back());
                        queue_.pop_back();
                        thread->do_work(std::move(work_data));

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


    size_t max_thread_count_;
    bool dead_;
    std::vector<std::shared_ptr<d_::WorkerThread>> threads_;
    std::vector<d_::WorkData> queue_;
    VT::Event has_queue_;
    VT::Event has_free_threads_;
    mutable std::mutex lock_;
    std::thread thread_;
};


VT::ThreadPool::ThreadPool(size_t thread_count, size_t max_thread_count)
    : d(new Impl(max_thread_count))
{
    set_thread_count(thread_count);
}


VT::ThreadPool::~ThreadPool()
{
    try
    {
        wait_for_all();  // wait for work to finish
    }
    catch (const std::exception&)
    {
        // some thread termination failed, ignoring...
    }

    delete d;
}


size_t VT::ThreadPool::thread_count() const
{
    std::lock_guard<std::mutex> l(d->lock_);
    return d->threads_.size();
}


size_t VT::ThreadPool::max_thread_count() const
{
    std::lock_guard<std::mutex> l(d->lock_);
    return d->max_thread_count_;
}


void VT::ThreadPool::set_thread_count(size_t count)
{
    assert(count <= d->max_thread_count_);
    std::lock_guard<std::mutex> l(d->lock_);
    d->adjust_thread_count(count);
}


void VT::ThreadPool::set_max_thread_count(size_t count)
{
    assert(count >= 1);
    std::lock_guard<std::mutex> l(d->lock_);
    d->max_thread_count_ = count;
    if (d->max_thread_count_ < d->threads_.size())
        d->adjust_thread_count(d->max_thread_count_);
}


std::future<void> VT::ThreadPool::run(const std::function<void()>& work)
{
    std::lock_guard<std::mutex> l(d->lock_);

    std::promise<void> promise;
    auto fut = promise.get_future();
    d_::WorkData work_data(work, std::move(promise));

    // do we have free threads?
    auto thread = d->free_thread();
    if (thread)
    {
        thread->do_work(std::move(work_data));
    }
    else if (d->threads_.size() < d->max_thread_count_)  // we still have free slots
    {
        d->add_thread();
        d->threads_.back()->do_work(std::move(work_data));
    }
    else  // queue that bitch
    {
        d->queue_.push_back(std::move(work_data));
        d->has_queue_.signal();
    }

    return fut;
}


bool VT::ThreadPool::wait_for_all(int timeout_ms)
{
    Timer timer;

    for (;;)
    {
        std::shared_ptr<d_::WorkerThread> thread;
        bool queue_empty = false;

        {
            std::lock_guard<std::mutex> l(d->lock_);

            queue_empty = d->queue_.empty();

            for (auto it = d->threads_.begin(); it != d->threads_.end(); ++it)
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
