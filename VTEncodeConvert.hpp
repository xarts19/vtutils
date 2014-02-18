#pragma once

#include "VTCompilerSpecific.h"

#include <string>

namespace VT
{
    std::wstring UTF8ToWstring(const std::string& input);
    std::wstring UTF8ToWstring(const char* input);
    std::string  WstringToUTF8(const std::wstring& input);
    std::string  WstringToUTF8(const wchar_t* input);
    std::wstring AnsiToWstring(const std::string& input, const char* locale = ".ACP");
    std::string  WstringToAnsi(const std::wstring& input, const char* locale = ".ACP");
}

#ifdef COMPILER_MSVC
    #define VT_TO_FILENAME_ENCODING(filename) (filename)
#else
    #define VT_TO_FILENAME_ENCODING(filename) (VT::WstringToUTF8(filename))
#endif
