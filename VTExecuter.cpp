#include "VTExecuter.h"

#include "VTEvent.hpp"

#include <functional>
#include <thread>
#include <iostream>

struct VT::Executer::impl
{
    impl(const std::function<void()>& update_fnc, int update_interval_ms)
        : interval_ms_(update_interval_ms)
        , fnc_(update_fnc)
        , thread_(std::bind(std::mem_fn(&Executer::impl::worker), this))
    { }

    ~impl()
    {
        stop_.signal();
        thread_.join();
    }

    void worker()
    {
        if (interval_ms_ == 0 || !fnc_)
            return;

        while (!stop_.wait(interval_ms_))
        {
            try
            {
                fnc_();
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Exception in VT::Executer thread: " << ex.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Uknown error in VT::Executer thread." << std::endl;
            }
        }
    }

    int interval_ms_;
    VT::Event stop_;
    std::function<void()> fnc_;
    std::thread thread_;
};

VT::Executer::Executer(const std::function<void()>& call_fnc, int call_interval_ms)
    : d(new impl(call_fnc, call_interval_ms))
{
}

VT::Executer::~Executer()
{
    delete d;
}

