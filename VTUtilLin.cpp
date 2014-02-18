#include "VTUtil.h"

#include "VTEncodeConvert.hpp"

#include <string.h>
#include <limits.h>
#include <unistd.h>

int VT::last_error()
{
    return errno;
}

std::string VT::strerror(int err_code)
{
    char buf[256];
    char* msg = strerror_r(err_code, buf, 256);
    return std::string(msg);
}

std::wstring VT::executable_path()
{
    const ssize_t size = 512;
    char buffer[size];
    ssize_t ret = readlink("/proc/self/exe", buffer, size);

    if (ret == -1)
        VT::create_system_exception("Failed to get current executable path");
    else
        buffer[(ret < size ? ret : size - 1)] = '\0';

    *(strrchr(buffer, '/') + 1) = '\0'; // Remove executable name
    return VT::UTF8ToWstring(buffer);
}

std::wstring VT::hostname()
{
    char buf[HOST_NAME_MAX];
    int ret = gethostname(buf, HOST_NAME_MAX);
    if (ret != -1)
        return VT::UTF8ToWstring(buf);

    return L"";
}
