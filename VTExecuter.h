#pragma once

#include "VTUtil.h"

#include <functional>

namespace VT
{
    class Executer
    {
    public:
        Executer(const std::function<void()>& fnc, int call_interval_ms);
        ~Executer();

    private:
        VT_DISABLE_COPY(Executer);

        struct impl;
        impl* d;
    };
}
