#include "VTUtil.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <assert.h>

std::string VT::Utils::human_readable_size(unsigned long long size)
{
    double dsize = static_cast<double>(size);
    unsigned int i = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    while (dsize > 1024)
    {
        dsize /= 1024;
        ++i;
    }
    
    assert(i < sizeof(units) / sizeof(units[0]));

    std::stringstream ss;
    ss << std::fixed << std::setprecision(i) << dsize << " " << units[i];
    return ss.str();
}
