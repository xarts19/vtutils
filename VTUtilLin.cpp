#include "VTUtil.h"

#include <string.h>
#include <sstream>


std::string VT::strerror(int err_code)
{
    char buf[256];
    char* msg = strerror_r(err_code, buf, 256);
    return std::string(msg);
}

