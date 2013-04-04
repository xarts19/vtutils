#include "VTFileUtil.h"

#include "VTUtil.h"

#include <windows.h>

std::string VT::full_path(const std::string& filename)
{
    WCHAR buffer[4096];

    DWORD retval = GetFullPathNameW(VT::convert(filename).c_str(), 
        4096, buffer, nullptr);

    if (retval == 0) 
    {
        throw std::runtime_error(VT::strerror(GetLastError()));
    }
    else
    {
        return VT::convert(buffer);
    }
}


std::string VT::folder_name(const std::string& filename)
{
    auto pos = filename.find_last_of('\\');
    if (pos != filename.npos)
        return filename.substr(0, pos + 1);
    else
        return filename;
}
