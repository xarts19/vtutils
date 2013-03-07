#include "VTTimer.h"

#include <stdexcept>
#include <sstream>
#include <assert.h>

#include <sys/time.h>

unsigned long long get_ms_since_epoch()
{
    timeval t;
    int ret = gettimeofday(&t, NULL);
    
    assert(ret == 0 && "gettimeofday returned error");
    if (ret != 0)
    {
        std::stringstream err;
        err << "gettimeofday returned error " << ret;
        throw std::runtime_error(err.str());
    }
    
    return t.tv_sec * 1000000LL + t.tv_sec;
}

VT::Timer::Timer(bool start_) : 
    counter_start_(0)
{
    if (start_)
        start();
}

void VT::Timer::start()
{
    counter_start_ = get_ms_since_epoch();
}

double VT::Timer::time_elapsed_s() const
{
    unsigned long long current_time = get_ms_since_epoch();
    return (current_time - counter_start_) / 1.0e6;
}
