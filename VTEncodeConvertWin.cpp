#include "VTEncodeConvert.hpp"

#include "VTUtil.h"

#include <string>
#include <locale>

#include <codecvt>

namespace VT
{

    std::wstring UTF8ToWstring( const std::string& input )
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
        return conv.from_bytes(input);
    }

    std::wstring UTF8ToWstring( const char* input )
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
        return conv.from_bytes(input);
    }

    std::string WstringToUTF8( const std::wstring& input )
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
        return conv.to_bytes(input);
    }

    std::string WstringToUTF8( const wchar_t* input )
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
        return conv.to_bytes(input);
    }

    std::wstring AnsiToWstring( const std::string& input, const char* locale /*= ".ACP" */ )
    {
        wchar_t buf[8192] = {0};

        char *locale_old = setlocale(LC_CTYPE, NULL);

        setlocale(LC_CTYPE, locale);
        mbstowcs(buf, input.c_str(), ARRAY_SIZE(buf));
        setlocale(LC_CTYPE, locale_old);

        return buf;
    }

    std::string WstringToAnsi( const std::wstring& input, const char* locale /*= ".ACP" */ )
    {
        char buf[8192] = {0};

        char *locale_old = setlocale(LC_CTYPE, NULL);

        setlocale(LC_CTYPE, locale);
        wcstombs(buf, input.c_str(), ARRAY_SIZE(buf));
        setlocale(LC_CTYPE, locale_old);

        return buf;
    }
}
