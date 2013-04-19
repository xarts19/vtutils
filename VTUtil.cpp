#include "VTUtil.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <assert.h>

std::string VT::Utils::human_readable_size(unsigned long long size, int precision)
{
    double dsize = static_cast<double>(size);
    unsigned int unit = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    while (dsize > 1024)
    {
        dsize /= 1024;
        ++unit;
    }
    
    assert(unit < sizeof(units) / sizeof(units[0]));

    if (precision == -1)
    {
        int i = 0;
        double delta = (dsize - int(dsize)) * 10;
        while (i < 5 && delta > 3)
        {
            delta = (delta - int(delta)) * 10;
            ++i;
        }
        precision = i;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << dsize << " " << units[unit];
    return ss.str();
}
