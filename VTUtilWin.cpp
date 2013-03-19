#include "VTUtil.h"

#include <Windows.h>

std::string VT::strerror(int err_code)
{
    std::string msg = "Failed to format error message";
    char* errorText = NULL;

    FormatMessage(
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
        (LPTSTR)&errorText,        // output
        0,                         // minimum size for output buffer
        NULL );

    if (NULL != errorText)
    {
        msg = errorText;

        // release memory allocated by FormatMessage()
        LocalFree( errorText );
        errorText = NULL;
    }

    return msg;
}

