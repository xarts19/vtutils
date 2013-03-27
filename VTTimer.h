#pragma once

namespace VT
{
    class Timer
    {
    public:
        explicit Timer(bool start = true);
        void start();
        double time_elapsed_s() const;

    private:
#ifdef _MSC_VER
        double              PCFreq_;
#endif
        unsigned long long  counter_start_;
    };
}
