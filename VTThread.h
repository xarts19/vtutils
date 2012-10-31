#pragma once

class VTThread
{
public:
    VTThread();
    ~VTThread();

    void start();
    bool join( int timeout_millis = -1 );
    bool isRunning();
    bool isFinished();
    virtual void run() = 0;

private:
    typedef void* HANDLE;

    //prevent copying
    VTThread( const VTThread& );
    VTThread& operator=( const VTThread& );

    static unsigned long __stdcall thread_start( void* params );

private:
    HANDLE thread_handle;
    int state;
};
