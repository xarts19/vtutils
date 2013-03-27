#pragma once

namespace VT
{
    // Event is manual-reset
    class Event
    {
    public:
        Event();
        ~Event();

        bool signal();
        bool reset();
        
        // return true if event is signaled and false on timeout
        bool wait(int timeout_ms = -1);

        bool is_signaled();

    private:
        Event(const Event&);
        Event& operator=(const Event&);

        struct Impl;
        Impl* pimpl_;
    };
}
