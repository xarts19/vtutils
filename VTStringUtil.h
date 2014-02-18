#pragma once
#include "VTUtil.h"

#include "VTEncodeConvert.hpp"

#include <string>
#include <sstream>

namespace VT
{
    template <typename tstring, typename Container>
    void tsplit(const tstring& str,
                Container*     result,
                const tstring& delimiters)
    {
        size_t current;
        size_t next = static_cast<size_t>(-1);
        do
        {
            next = str.find_first_not_of( delimiters, next + 1 );
            if (next == tstring::npos) break;
            next -= 1;

            current = next + 1;
            next = str.find_first_of( delimiters, current );
            result->push_back( str.substr( current, next - current ) );
        }
        while (next != tstring::npos);
    }


    // delimiters is a string of delimiting characters of length 1
    // do not include empty strings between consecutive delimiters
    template <typename Container>
    void split(const std::string& str,
               Container*         result,
               const std::string& delimiters = " ")
    {
        tsplit(str, result, delimiters);
    }

    template <typename Container>
    void split(const std::wstring& str,
               Container*          result,
               const std::wstring& delimiters = L" ")
    {
        tsplit(str, result, delimiters);
    }

    // return string without any chars from 'whitespace' at the beginning or at the end
    inline std::string trim(const std::string& str, const std::string& whitespace = " ")
    {
        const size_t strBegin = str.find_first_not_of(whitespace);
        if (strBegin == std::string::npos)
            return ""; // no content

        const size_t strEnd = str.find_last_not_of(whitespace);
        const size_t strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

    bool starts_with(const std::string& str, const std::string& prefix);
    bool starts_with(const std::wstring& str, const std::wstring& prefix);
    bool ends_with(const std::string& str, const std::string& suffix);
    bool ends_with(const std::wstring& str, const std::wstring& suffix);

    std::string to_lower(std::string str);

    // Simple ToString convert
    template<typename Ch, typename T>
    std::basic_string<Ch> to_string(T val, bool hex = false)
    {
        std::basic_stringstream<Ch> sstream;
        if(hex)
            sstream << std::hex;

        sstream << val;
        return sstream.str();
    }

    // Convert to char or wchar_t depending on specialization
    template <typename Ch>
    inline std::basic_string<Ch> ensure_tchar(const char* ptr);

    template <typename Ch>
    inline std::basic_string<Ch> ensure_tchar(const wchar_t* ptr);

    template <>
    inline std::basic_string<char> ensure_tchar(const char* ptr)
    {
        return ptr;
    }

    template <>
    inline std::basic_string<wchar_t> ensure_tchar(const char* ptr)
    {
        return VT::UTF8ToWstring(ptr);
    }

    template <>
    inline std::basic_string<char> ensure_tchar(const wchar_t* ptr)
    {
        return VT::WstringToUTF8(ptr);
    }

    template <>
    inline std::basic_string<wchar_t> ensure_tchar(const wchar_t* ptr)
    {
        return ptr;
    }



    template <typename Ch>
    inline Ch ensure_tchar(char ch);

    template <typename Ch>
    inline Ch ensure_tchar(wchar_t ch);

    template <>
    inline char ensure_tchar(char ch)
    {
        return ch;
    }

    template <>
    inline wchar_t ensure_tchar(char ch)
    {
        return VT::UTF8ToWstring(std::string(1, ch)).front();
    }

    template <>
    inline char ensure_tchar(wchar_t ch)
    {
        return VT::WstringToUTF8(std::wstring(1, ch)).front();
    }

    template <>
    inline wchar_t ensure_tchar(wchar_t ch)
    {
        return ch;
    }
}


