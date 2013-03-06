#include "VTThread.h"
#include 

#include <stdexcept>
#include <string>

#include <unistd.h>
#include <limits.h>
#include <pthread.h>

#include "VTCriticalSection.h"

#include <stdexcept>
#include <string>

#include <assert.h>
#include <Windows.h>


struct VT::Thread::Impl
{
    Impl()
        : thread_handle()
        , state(detail_::ThreadState::NotStarted)
        , state_lock()
    { }

    static unsigned long WINAPI thread_start(void* params);

    pthread_t  thread_handle;
    int        state;
    mutable VT::CriticalSection state_lock;
};


unsigned long WINAPI VT::Thread::Impl::thread_start( void* params )
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
    return 0;
}


void VT::Thread::sleep(int time_ms)
{
    usleep(time_ms * 1000);
}

unsigned long VT::Thread::current_thread_id()
{
    return pthread_self();
}

VT::Thread::Thread()
    : pimpl_(new Impl)
{
}

VT::Thread::~Thread()
{
    if ( pimpl_->thread_handle != NULL )
        CloseHandle( pimpl_->thread_handle );
}

void VT::Thread::start()
{
    {
        VT::CSLocker lock(pimpl_->state_lock);
        if (pimpl_->state != detail_::ThreadState::NotStarted)
            throw std::runtime_error("Thread already started");
        pimpl_->state = detail_::ThreadState::Init;
    }

    HANDLE h = CreateThread( NULL, 0, &Impl::thread_start, this, 0, NULL );

    if ( h == NULL )
    {
        throw std::runtime_error( "Failed to create thread. Error: " + get_win_error_msg() );
    }

    pimpl_->thread_handle = h;
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
    return GetThreadId(pimpl_->thread_handle);
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


