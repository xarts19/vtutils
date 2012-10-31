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

enum State {Running, Waiting, Finished, NotStarted};

unsigned long WINAPI VTThread::thread_start( void* params )
{
    VTThread* t = reinterpret_cast<VTThread*>( params );
    t->run();
    t->state = Finished;
    return 0;
}

VTThread::VTThread() : thread_handle( NULL ), state( NotStarted )
{
}

VTThread::~VTThread()
{
    if ( state != NotStarted )
    { CloseHandle( thread_handle ); }
}

void VTThread::start()
{
    state = Running;
    HANDLE h = CreateThread( NULL, 0, &thread_start, this,  0, NULL );

    if ( h == NULL )
    {
        throw std::runtime_error( "Failed to create thread. Error: " + get_win_error_msg() );
    }

    thread_handle = h;
}

bool VTThread::join( int timeout_millis )
{
    assert( timeout_millis >= -1 );

    if ( state == NotStarted || state == Finished )
    { return true; }

    state = Waiting;
    int result = WaitForSingleObject( thread_handle, ( timeout_millis == -1 ? INFINITE : timeout_millis ) );

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

bool VTThread::isRunning()
{
    return ( state == Running );
}

bool VTThread::isFinished()
{
    return ( state == Finished );
}