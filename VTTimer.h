#pragma once

#include <chrono>

namespace VT
{
    class Timer
    {
    public:
        explicit Timer(bool start = true);
        void start();
        double time_elapsed_s() const;
        std::chrono::milliseconds time_elapsed() const
        {
            return std::chrono::milliseconds(static_cast<int64_t>(time_elapsed_s() * 1000));
        }

    private:
#ifdef _MSC_VER
        double              PCFreq_;
#endif
        unsigned long long  counter_start_;
    };
}
