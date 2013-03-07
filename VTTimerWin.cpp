#include "VTTimer.h"

#include <stdexcept>
#include <sstream>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

unsigned long long query_performance_counter()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceCounter(&li))
    {
        std::stringstream err;
        err << "Couldn't get timer info: QueryPerformanceCounter failed with error " << GetLastError();
        throw std::runtime_error(err.str());
    }
    return li.QuadPart;
}

VT::Timer::Timer(bool start_) : 
    PCFreq_(0.0),
    counter_start_(0)
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
    {
        std::stringstream err;
        err << "Couldn't init timer frequency: QueryPerformanceFrequency failed with error " << GetLastError();
        throw std::runtime_error(err.str());
    }
    PCFreq_ = double(li.QuadPart);

    if (start_)
        start();
}

void VT::Timer::start()
{
    counter_start_ = query_performance_counter();
}

double VT::Timer::time_elapsed_s() const
{
    unsigned long long current_time = query_performance_counter();
    return double(current_time - counter_start_) / PCFreq_;
}
