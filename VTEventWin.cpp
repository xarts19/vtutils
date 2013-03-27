#include "VTEvent.h"

#include <Windows.h>

#include <assert.h>
#include <stdexcept>
#include <sstream>


struct VT::Event::Impl
{
    HANDLE event_data;
};


VT::Event::Event()
    : pimpl_(new Impl)
{
    pimpl_->event_data = CreateEvent(NULL, TRUE, FALSE, NULL);
}


VT::Event::~Event()
{
    CloseHandle(pimpl_->event_data);
    delete pimpl_;
}


bool VT::Event::signal()
{
    return 0 != SetEvent(pimpl_->event_data);
}


bool VT::Event::reset()
{
    return 0 != ResetEvent(pimpl_->event_data);
}


bool VT::Event::wait(int timeout_ms)
{
    assert(timeout_ms >= -1);
    if (timeout_ms == -1)
        timeout_ms = INFINITE;

    DWORD state = WaitForSingleObject(pimpl_->event_data, timeout_ms);

    if ( state == WAIT_TIMEOUT )
    {
        return false;
    }
    else if ( state == WAIT_OBJECT_0 )
    {
        return true;
    }
    else // if state == WAIT_FAILED or other
    {
        std::stringstream ss;
        ss << "Waiting on event failed: WaitForSingleObject returned " << GetLastError();
        throw std::runtime_error(ss.str());
    }
}


bool VT::Event::is_signaled()
{
    return wait(0);
}
