#include "VTThread.h"

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

enum State {Running, Waiting, Finished, NotStarted, Init};

unsigned long WINAPI VT::Thread::thread_start( void* params )
{
    Thread* t = reinterpret_cast<Thread*>( params );
    {
        VT::CSLocker lock(t->state_lock_);
        t->state_ = Running;
    }
    t->run();
    {
        VT::CSLocker lock(t->state_lock_);
        t->state_ = Finished;
    }
    return 0;
}

VT::Thread::Thread()
    : thread_handle_(NULL)
    , state_(NotStarted)
    , state_lock_()
{
}

VT::Thread::~Thread()
{
    if ( thread_handle_ != NULL )
        CloseHandle( thread_handle_ );
}

void VT::Thread::start()
{
    {
        VT::CSLocker lock(state_lock_);
        if (state_ != NotStarted)
            throw std::runtime_error("Thread already started");
        state_ = Init;
    }

    HANDLE h = CreateThread( NULL, 0, &thread_start, this,  0, NULL );

    if ( h == NULL )
    {
        throw std::runtime_error( "Failed to create thread. Error: " + get_win_error_msg() );
    }

    thread_handle_ = h;
}

bool VT::Thread::join( int timeout_millis )
{
    assert( timeout_millis >= -1 );

    {
        VT::CSLocker lock(state_lock_);
        if ( state_ == NotStarted || state_ == Finished )
            return true;
    }

    int result = WaitForSingleObject( thread_handle_, ( timeout_millis == -1 ? INFINITE : timeout_millis ) );

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

bool VT::Thread::isRunning()
{
    VT::CSLocker lock(state_lock_);
    return ( state_ == Running || state_ == Init );
}

bool VT::Thread::isFinished()
{
    VT::CSLocker lock(state_lock_);
    return ( state_ == Finished );
}