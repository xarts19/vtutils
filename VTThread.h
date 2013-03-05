#pragma once

#include "VTCriticalSection.h"

namespace VT
{

    class Thread
    {
    public:
        Thread();
        ~Thread();

        void start();
        bool join( int timeout_millis = -1 );
        bool isRunning();
        bool isFinished();
        virtual void run() = 0;

    private:
        typedef void* HANDLE;

        //prevent copying
        Thread( const Thread& );
        Thread& operator=( const Thread& );

        static unsigned long __stdcall thread_start( void* params );

    private:
        HANDLE thread_handle_;
        int state_;
        VT::CriticalSection state_lock_;
    };

};
