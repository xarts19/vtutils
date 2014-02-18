#include "VTUtil.h"

#include "VTEncodeConvert.hpp"
#include "VTFileUtil.h"

#include <sstream>

#include <Windows.h>

int VT::last_error()
{
    return GetLastError();
}


std::string VT::strerror(int err_code)
{
    std::string msg;
    LPWSTR errorText = NULL;

    FormatMessageW(
        // use system message tables to retrieve error text
        FORMAT_MESSAGE_FROM_SYSTEM
        // allocate buffer on local heap for error text
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        // Important! will fail otherwise, since we're not
        // (and CANNOT) pass insertion parameters
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,                      // unused with FORMAT_MESSAGE_FROM_SYSTEM
        err_code,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (LPWSTR)&errorText,        // output
        0,                         // minimum size for output buffer
        NULL );

    if (NULL != errorText)
    {
        msg = VT::WstringToUTF8(errorText);

        // release memory allocated by FormatMessage()
        LocalFree(errorText);
    }
    else
    {
        std::stringstream ss;
        ss << err_code;
        msg = ss.str();
    }

    return msg;
}


#ifdef VT_USE_COM

#include <comdef.h>

std::string VT::com_error(long hresult)
{
    _com_error err(hresult);
    LPCTSTR errMsg = err.ErrorMessage();
    return VT::WstringToUTF8(errMsg);
}

#endif


std::wstring VT::executable_path()
{
    const size_t size = 512;
    wchar_t filename[size];
    auto ret = GetModuleFileNameW(0, filename, size);

    if (ret == 0)
        VT::create_system_exception("Failed to get current executable path");

    std::wstring res = filename;
    auto pos = res.rfind(VT_SLASH);

    if (pos == res.npos)
        VT::create_system_exception("Backslash not found in executable path");

    return res.substr(0, pos + 1);  // Remove exe name
}

std::wstring VT::hostname()
{
    unsigned long bufferLen = 64;
    std::unique_ptr<wchar_t[]> hostName(new wchar_t[bufferLen]);

    if(::GetComputerNameEx(ComputerNamePhysicalDnsHostname, hostName.get(), &bufferLen) != 0)
    {
        return hostName.get();
    }
    else if(GetLastError() == ERROR_MORE_DATA)
    {
        hostName = std::unique_ptr<wchar_t[]>(new wchar_t[bufferLen]);

        if(::GetComputerNameEx(ComputerNamePhysicalDnsHostname, hostName.get(), &bufferLen) != 0)
            return hostName.get();
    }

    return L"";
}

