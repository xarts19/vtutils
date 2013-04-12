#include "VTFileUtil.h"

#include "VTUtil.h"

#include <stdexcept>

#include <stdlib.h>
#include <limits.h>


std::string VT::full_path(const std::string& filename)
{
    char buffer[PATH_MAX];

    char* ptr = realpath(filename.c_str(), buffer);

    if (ptr == 0) 
    {
        throw std::runtime_error(VT::strerror(errno));
    }
    else
    {
        return buffer;
    }
}
