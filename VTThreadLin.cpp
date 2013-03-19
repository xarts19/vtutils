#include "VTThread.h"

#include "VTUtil.h"

#include <stdexcept>
#include <string>
#include <sstream>

#include <assert.h>
#include <limits.h>

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


struct VT::Thread::Impl
{
    Impl()
        : thread_handle()
        , state(detail_::ThreadState::NotStarted)
        , finished_cond()
        , lock()
    {
        int ret = pthread_cond_init(&finished_cond, nullptr);
        if (ret != 0)
            throw std::runtime_error("Failed to init pthread_cond_t finished_cond variable");
        ret = pthread_mutex_init(&lock, nullptr);
        if (ret != 0)
            throw std::runtime_error("Failed to init pthread_mutex_t finished_lock variable");
    }
    
    ~Impl()
    {
        int ret = pthread_cond_destroy(&finished_cond);
        assert(ret == 0);
        ret = pthread_mutex_destroy(&lock);
        assert(ret == 0);
    }

    // function of type THREAD_START_LINUX
    static void* thread_start(void* params);

    pthread_t  thread_handle;
    int        state;

    pthread_cond_t  finished_cond;
    pthread_mutex_t lock;
};


void* VT::Thread::Impl::thread_start( void* params )
{
    Thread* t = reinterpret_cast<Thread*>( params );

    int ret = pthread_mutex_lock(&t->pimpl_->lock);
    assert(ret == 0);

    t->pimpl_->state = detail_::ThreadState::Running;

    ret = pthread_mutex_unlock(&t->pimpl_->lock);
    assert(ret == 0);
    
    t->run();

    ret = pthread_mutex_lock(&t->pimpl_->lock);
    assert(ret == 0);

    t->pimpl_->state = detail_::ThreadState::Finished;
    ret = pthread_cond_signal(&t->pimpl_->finished_cond);
    assert(ret == 0);

    ret = pthread_mutex_unlock(&t->pimpl_->lock);
    assert(ret == 0);
    
    return nullptr;
}


void VT::Thread::sleep(int time_ms)
{
    usleep(time_ms * 1000);
}

unsigned long VT::Thread::current_thread_id()
{
    // lucky me, thread_t == unsigned long, just as in Windows
    // where ThreadId is DWORD which is unsigned long
    return pthread_self();
}

VT::Thread::Thread()
    : pimpl_(new Impl)
{
}

VT::Thread::~Thread()
{
}

void VT::Thread::start()
{
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);

    assert(pimpl_->state == detail_::ThreadState::NotStarted && "Attempt to start VT::Thread twice");

    pimpl_->state = detail_::ThreadState::Init;

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    ret = pthread_create(&pimpl_->thread_handle, nullptr, &Impl::thread_start, this);
    
    if ( ret != 0 )
        throw std::runtime_error( "Failed to create thread. Error: " + VT::strerror(ret) );
}

bool VT::Thread::join( int timeout_millis )
{
    assert( timeout_millis >= -1 && "Incorrect timeout value for VT::Thread::join" );
    
    struct timespec timeout;
    
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);

    assert(pimpl_->state != detail_::ThreadState::NotStarted && "Attempt to join not started VT::Thread");
    
    if (timeout_millis != -1)
    {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_millis / 1000;
        timeout.tv_nsec += (timeout_millis % 1000) * 1000000;
        if (timeout.tv_nsec > 1000000000)
        {
            timeout.tv_sec += 1;
            timeout.tv_nsec -= 1000000000;
        }
    }
    
    int wait_ret = 0;
    while (pimpl_->state != detail_::ThreadState::Finished && wait_ret == 0)
    {
        if (timeout_millis != -1)
            wait_ret = pthread_cond_timedwait(&pimpl_->finished_cond, &pimpl_->lock, &timeout);
        else
            wait_ret = pthread_cond_wait(&pimpl_->finished_cond, &pimpl_->lock);
    }

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    if (wait_ret == 0)
        return true;  // thread finished
    else if (wait_ret == ETIMEDOUT)
        return false;
    else
        assert(0 && "Error in pthread_cond_timedwait other than timeout");
}

bool VT::Thread::isRunning() const
{
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);

    bool cond = pimpl_->state == detail_::ThreadState::Running || pimpl_->state == detail_::ThreadState::Init;

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    return cond;
}

bool VT::Thread::isFinished() const
{
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);

    bool cond = pimpl_->state == detail_::ThreadState::Finished;

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    return cond;
}

unsigned long VT::Thread::id() const
{
    // same comment as in VT::Thread::current_thread_id
    return pimpl_->thread_handle;
}
