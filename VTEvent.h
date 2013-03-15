#pragma once

// TODO: Linux Event class

namespace VT
{
    class Event
    {
    public:
        Event();
        ~Event();

        bool signal();
        bool reset();
        bool wait(int timeout_ms = -1);

    private:
        Event(const Event&);
        Event& operator=(const Event&);

        struct Impl;
        Impl* pimpl_;
    };
}