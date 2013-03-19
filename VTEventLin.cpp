#include "VTEvent.h"

#include <assert.h>

#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

struct VT::Event::Impl
{
    Impl()
        : cond()
        , lock()
        , signaled(false)
    {
        int ret = pthread_cond_init(&cond, nullptr);
        if (ret != 0)
            throw std::runtime_error("Failed to init pthread_cond_t finished_cond variable");
        ret = pthread_mutex_init(&lock, nullptr);
        if (ret != 0)
            throw std::runtime_error("Failed to init pthread_mutex_t finished_lock variable");
    }
    
    ~Impl()
    {
        int ret = pthread_cond_destroy(&cond);
        assert(ret == 0);
        ret = pthread_mutex_destroy(&lock);
        assert(ret == 0);
    }
    
    pthread_cond_t  cond;
    pthread_mutex_t lock;
    bool            signaled;
};

VT::Event::Event()
    : pimpl_(new Impl)
{
}

VT::Event::~Event()
{
}

bool VT::Event::signal()
{
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);

    pimpl_->signaled = true;
    ret = pthread_cond_broadcast(&pimpl_->cond);
    assert(ret == 0);

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    return true;
}

bool VT::Event::reset()
{
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);

    pimpl_->signaled = false;

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    return true;
}

bool VT::Event::wait(int timeout_ms)
{
    assert( timeout_ms >= -1 && "Incorrect timeout value for VT::Thread::join" );
    
    struct timespec timeout;
    
    int ret = pthread_mutex_lock(&pimpl_->lock);
    assert(ret == 0);
    
    if (timeout_ms != -1)
    {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_ms / 1000;
        timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (timeout.tv_nsec > 1000000000)
        {
            timeout.tv_sec += 1;
            timeout.tv_nsec -= 1000000000;
        }
    }
    
    int wait_ret = 0;
    while (pimpl_->signaled != true && wait_ret == 0)
    {
        if (timeout_ms != -1)
            wait_ret = pthread_cond_timedwait(&pimpl_->cond, &pimpl_->lock, &timeout);
        else
            wait_ret = pthread_cond_wait(&pimpl_->cond, &pimpl_->lock);
    }

    ret = pthread_mutex_unlock(&pimpl_->lock);
    assert(ret == 0);
    
    if (wait_ret == 0)
        return true;  // event signaled
    else if (wait_ret == ETIMEDOUT)
        return false;
    else
        assert(0 && "Error in pthread_cond_timedwait other than timeout");
    
    return false;  // unreachable
}

