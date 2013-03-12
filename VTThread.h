#pragma once


namespace VT
{
    namespace detail_
    {
        namespace ThreadState {
            enum ThreadState {Running, Finished, NotStarted, Init};
        }
    }

    class Thread
    {
    public:
        Thread();
        ~Thread();

        void start();
        bool join( int timeout_millis = -1 );  // -1 means INFINITE
        bool isRunning() const;
        bool isFinished() const;

        unsigned long id() const;
        
        static void sleep(int time_ms);
        static unsigned long current_thread_id();

    private:
        // prevent copying
        Thread( const Thread& );
        Thread& operator=( const Thread& );

        // override this function to execute on thread
        virtual void run() = 0;

        struct Impl;
        Impl* pimpl_;
    };

}
