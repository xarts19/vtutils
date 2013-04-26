#include "VTFuture.h"

VT::Promise::Promise()
    : event_(new VT::Event())
{

}


VT::Promise::Promise(const Promise& other)
    : event_(other.event_)
{
}


VT::Promise& VT::Promise::operator=(const Promise& rhs)
{
    event_ = rhs.event_;
    return *this;
}


void VT::Promise::set_done()
{
    event_->signal();
}


VT::Future VT::Promise::get_future() const
{
    Future f;
    f.event_ = event_;
    return f;
}


VT::Future::Future(const Future& other)
    : event_(other.event_)
{
}


VT::Future& VT::Future::operator=(const Future& rhs)
{
    event_ = rhs.event_;
    return *this;
}


bool VT::Future::wait(int timeout_ms)
{
    if (!event_)
        throw std::runtime_error("Invalid future");
    return event_->wait(timeout_ms);
}


bool VT::Future::valid() const
{
    return event_;
}
