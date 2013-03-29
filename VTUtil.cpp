#include "VTUtil.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <assert.h>

std::string VT::Utils::human_readable_size(unsigned long long size)
{
    double dsize = static_cast<double>(size);
    int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    while (dsize > 1024)
    {
        dsize /= 1024;
        ++i;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(i) << dsize << " " << units[i];
    return ss.str();
}


std::string VT::Utils::human_readable_size(long long size)
{
    assert(size >= 0);
    return human_readable_size(static_cast<unsigned long long>(size));
}
