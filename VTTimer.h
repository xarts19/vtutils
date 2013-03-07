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
        double              PCFreq_;
        unsigned long long  counter_start_;
    };
}