#pragma once

namespace VT
{

    // At least on Windows, it's safe to call enter on already owned lock
    class Lock
    {

    public:
        Lock();
        ~Lock();

        void enter();
        bool try_enter();
        void leave();

        // TODO:
        // pthread_mutex_timedlock
        // add timout to enter()
        // PTHREAD_MUTEX_RECURSIVE

    private: // non-copyable, non-movable
        Lock(const Lock&);
        Lock& operator=(const Lock&);

    private:
        struct Impl;
        Impl* data;
    };
    
    class Locker
    {
    public:
        explicit Locker(Lock& cs)
            : cs_(cs)
            , do_unlock_(true)
        {
            cs_.enter();
        }

        ~Locker()
        {
            if (do_unlock_)
                cs_.leave();
        }

        void lock()
        {
            do_unlock_ = true;
            cs_.enter();
        }

        void unlock()
        {
            do_unlock_ = false;
            cs_.leave();
        }
        
    private:
        Locker(const Locker&);
        Locker& operator=(const Locker&);
    
        Lock& cs_;
        bool do_unlock_;
    };

}
