#include "VTThread.h"

#include "VTCriticalSection.h"

#include <stdexcept>
#include <string>
#include <sstream>

#include <assert.h>
#include <limits.h>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>


std::string get_lin_error_msg(int code)
{
    switch (code)
    {
    case EAGAIN: return "Insufficient resources (EAGAIN)";
    case EINVAL: return "Invalid attr settings (EINVAL)";
    case EPERM: return "Insufficient permissions (EPREM)";
    default:
        {
            std::stringstream s;
            s << code;
            return "Unknown error: " + s.str();
        }
    }
}


struct VT::Thread::Impl
{
    Impl()
        : thread_handle()
        , state(detail_::ThreadState::NotStarted)
        , state_lock()
    { }

    // function of type THREAD_START_LINUX
    static void* thread_start(void* params);

    pthread_t  thread_handle;
    int        state;
    mutable VT::CriticalSection state_lock;
};


void* VT::Thread::Impl::thread_start( void* params )
{
    Thread* t = reinterpret_cast<Thread*>( params );
    {
        VT::CSLocker lock(t->pimpl_->state_lock);
        t->pimpl_->state = detail_::ThreadState::Running;
    }
    t->run();
    {
        VT::CSLocker lock(t->pimpl_->state_lock);
        t->pimpl_->state = detail_::ThreadState::Finished;
    }
    
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
    {
        VT::CSLocker lock(pimpl_->state_lock);
        if (pimpl_->state != detail_::ThreadState::NotStarted)
            throw std::runtime_error("Thread already started");
        pimpl_->state = detail_::ThreadState::Init;
    }

    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if (ret != 0)
        throw std::runtime_error( "Failed to init thread attributes" );
    
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    assert(ret == 0);
    
    ret = pthread_create(&pimpl_->thread_handle, &attr, &Impl::thread_start, this);
    
    ret = pthread_attr_destroy(&attr);
    if (ret != 0)
        ; // ignore

    if ( ret != 0 )
        throw std::runtime_error( "Failed to create thread. Error: " + get_lin_error_msg(ret) );
}

bool VT::Thread::join( int timeout_millis )
{
    assert( timeout_millis >= -1 );

    {
        VT::CSLocker lock(pimpl_->state_lock);
        if ( pimpl_->state == detail_::ThreadState::NotStarted || pimpl_->state == detail_::ThreadState::Finished )
            return true;
    }

    int result = WaitForSingleObject( pimpl_->thread_handle, ( timeout_millis == -1 ? INFINITE : timeout_millis ) );

    if ( result == WAIT_TIMEOUT )
    {
        return false;
    }
    else if ( result == WAIT_FAILED )
    {
        throw std::runtime_error( "Something went terribly wrong inside MyThread class. Failed to wait on running thread." );
    }
    else
    {
        return true;
    }
}

bool VT::Thread::isRunning() const
{
    VT::CSLocker lock(pimpl_->state_lock);
    return ( pimpl_->state == detail_::ThreadState::Running || pimpl_->state == detail_::ThreadState::Init );
}

bool VT::Thread::isFinished() const
{
    VT::CSLocker lock(pimpl_->state_lock);
    return ( pimpl_->state == detail_::ThreadState::Finished );
}

unsigned long VT::Thread::id() const
{
    // same comment as in VT::Thread::current_thread_id
    return pimpl_->thread_handle;
}


typedef void* (*THREAD_START_LINUX)(void* lpParameter);

const unsigned int Thread::T_INFINITE = UINT_MAX;

class ThreadLinux : public Thread
{
public:

	ThreadLinux(pthread_t thread): _thread(thread) {}
	~ThreadLinux() {}

	void join(unsigned int timeout)
	{
		pthread_join(_thread, NULL);
	}

private:
	pthread_t _thread;
};

Thread* Thread::create(	size_t stack_size,
						THREAD_START start_address,
						void* parameter,
						unsigned int creation_flags,
						unsigned int* thread_id)
{
	pthread_t thread;
	int fail = pthread_create(&thread, NULL, (THREAD_START_LINUX)start_address, parameter);
	if(fail)
	{
		return NULL;
	}
	else
	{
		Thread* t = new ThreadLinux(thread);
		return t;
	}
}

void Thread::Sleep(unsigned int time_millisecond)
{
	usleep(time_millisecond * 1000);
}


