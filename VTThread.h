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
        bool isRunning() const;
        bool isFinished() const;
        virtual void run() = 0;

        unsigned long id() const;
        
        static void sleep(int time_ms);
        static unsigned long current_thread_id();

    private:
        typedef void* HANDLE;

        //prevent copying
        Thread( const Thread& );
        Thread& operator=( const Thread& );

        static unsigned long __stdcall thread_start( void* params );

    private:
        HANDLE thread_handle_;
        int state_;
        mutable VT::CriticalSection state_lock_;
    };

};
