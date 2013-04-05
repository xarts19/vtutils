#include "VTThread.h"

#include "VTLock.h"
#include "VTUtil.h"

#include <stdexcept>
#include <string>

#include <assert.h>
#include <Windows.h>


struct VT::Thread::Impl
{
    Impl()
        : thread_handle(NULL)
        , state(detail_::ThreadState::NotStarted)
        , state_lock()
    { }

    static unsigned long WINAPI thread_start(void* params);

    HANDLE  thread_handle;
    int     state;
    mutable VT::Lock state_lock;
};


unsigned long WINAPI VT::Thread::Impl::thread_start( void* params )
{
    Thread* t = reinterpret_cast<Thread*>( params );
    {
        VT::Locker lock(t->pimpl_->state_lock);
        t->pimpl_->state = detail_::ThreadState::Running;
    }
    t->run();
    {
        VT::Locker lock(t->pimpl_->state_lock);
        t->pimpl_->state = detail_::ThreadState::Finished;
    }
    return 0;
}


void VT::Thread::sleep(int time_ms)
{
    Sleep(time_ms);
}

unsigned long VT::Thread::current_thread_id()
{
    return GetCurrentThreadId();
}

VT::Thread::Thread()
    : pimpl_(new Impl)
{
}

VT::Thread::~Thread()
{
    if (is_running())
        join();

    if ( pimpl_->thread_handle != NULL )
        CloseHandle( pimpl_->thread_handle );
}

void VT::Thread::start()
{
    {
        VT::Locker lock(pimpl_->state_lock);
        assert(pimpl_->state == detail_::ThreadState::NotStarted && "Attempt to start VT::Thread twice");
        pimpl_->state = detail_::ThreadState::Init;
    }

    HANDLE h = CreateThread( NULL, 0, &Impl::thread_start, this, 0, NULL );

    if ( h == NULL )
    {
        throw std::runtime_error("Failed to create thread. Error: " + VT::strerror(GetLastError()));
    }

    pimpl_->thread_handle = h;
}

bool VT::Thread::join(int timeout_millis)
{
    assert( timeout_millis >= -1 );
    assert( pimpl_->state != detail_::ThreadState::NotStarted && "Attempt to join not started VT::Thread");

    {
        VT::Locker lock(pimpl_->state_lock);
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
        throw std::runtime_error("Something went terribly wrong inside MyThread class. Failed to wait on running thread.");
    }
    else
    {
        return true;
    }
}

bool VT::Thread::is_running() const
{
    VT::Locker lock(pimpl_->state_lock);
    return ( pimpl_->state == detail_::ThreadState::Running || pimpl_->state == detail_::ThreadState::Init );
}

bool VT::Thread::is_finished() const
{
    VT::Locker lock(pimpl_->state_lock);
    return ( pimpl_->state == detail_::ThreadState::Finished );
}

unsigned long VT::Thread::id() const
{
    return GetThreadId(pimpl_->thread_handle);
}
