#include "VTThread.h"
#include "VTThread_p.h"

#include "VTCriticalSection.h"

#include <stdexcept>
#include <string>

#include <assert.h>
#include <Windows.h>


std::string get_win_error_msg()
{
    std::string msg;
    char* errorText = NULL;

    FormatMessage(
        // use system message tables to retrieve error text
        FORMAT_MESSAGE_FROM_SYSTEM
        // allocate buffer on local heap for error text
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        // Important! will fail otherwise, since we're not
        // (and CANNOT) pass insertion parameters
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,                      // unused with FORMAT_MESSAGE_FROM_SYSTEM
        GetLastError(),
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        ( LPTSTR )&errorText,      // output
        0,                         // minimum size for output buffer
        NULL );

    if ( NULL != errorText )
    {
        msg = errorText;

        // release memory allocated by FormatMessage()
        LocalFree( errorText );
        errorText = NULL;
    }

    return msg;
}

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
    if ( pimpl_->thread_handle != NULL )
        CloseHandle( pimpl_->thread_handle );
}

void VT::Thread::start()
{
    {
        VT::CSLocker lock(pimpl_->state_lock);
        assert(pimpl_->state == detail_::ThreadState::NotStarted && "Attempt to start VT::Thread twice");
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
    assert( pimpl_->state != detail_::ThreadState::NotStarted && "Attempt to join not started VT::Thread");

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
