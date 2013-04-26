#pragma once

#include "VTEvent.h"
#include "VTUtil.h"

#include <memory>

namespace VT { class Future; }

namespace VT
{
    class Promise
    {
    public:
        Promise();
        Promise(const Promise&);
        Promise& operator=(const Promise&);

        void set_done();
        Future get_future() const;

    private:
        std::shared_ptr<VT::Event> event_;
    };


    class Future
    {
    public:
        Future() {}
        Future(const Future&);
        Future& operator=(const Future&);

        // return true if task is done and false on timeout
        // if timeout == -1, waits infinitely
        bool wait(int timeout_ms = -1);
        bool valid() const;

    private:
        friend class Promise;

        std::shared_ptr<VT::Event> event_;
    };
}
