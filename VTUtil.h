#pragma once

#include <vector>
#include <algorithm>
#include <string>

#define VT_DISABLE_COPY(Class) Class(const Class &); Class & operator= (const Class &);
#define VT_UNUSED( x ) do { (void)sizeof(x); } while(0)

namespace VT
{
    // encoding remarks:
    /*
        * Do not use wchar_t or std::wstring in any place other than
          adjacent point to APIs accepting UTF-16.

        * Don't use _T("") or L"" UTF-16 literals (These should IMO
          be taken out of the standard, as a part of UTF-16 deprecation).

        * Don't use types, functions or their derivatives that are
          sensitive to the _UNICODE constant, such as LPTSTR or CreateWindow().

        * Yet, _UNICODE always defined, to avoid passing char* strings to WinAPI
          getting silently compiled

        * std::strings and char* anywhere in program are considered UTF-8 (if not said otherwise)

        * All my strings are std::string, though you can pass char* or string
          literal to convert(const std::string &).

        * only use Win32 functions that accept widechars (LPWSTR). Never those
          which accept LPTSTR or LPSTR. Pass parameters this way:

            ::SetWindowTextW(Utils::convert(someStdString or "string litteral").c_str())

        * With MFC strings:

            CString someoneElse; // something that arrived from MFC. 
                                 // Converted as soon as possible, before 
                                 // passing any further away from the API call:

            std::string s = str(boost::format("Hello %s\n") % Convert(someoneElse));
            AfxMessageBox(MfcUtils::Convert(s), _T("Error"), MB_OK);

        * Working with files, filenames and fstream on Windows:

            * Never pass std::string or const char* filename arguments to fstream
              family. MSVC STL does not support UTF-8 arguments, but has
              a non-standard extension which should be used as follows:

            * Convert std::string arguments to std::wstring with Utils::Convert:

                std::ifstream ifs(Utils::Convert("hello"),
                                  std::ios_base::in |
                                  std::ios_base::binary);

            * We'll have to manually remove the convert, when MSVC's attitude to fstream changes.

            * This code is not multi-platform and may have to be changed manually in the future

            * See fstream unicode research/discussion case 4215 for more info.

            * Never produce text output files with non-UTF8 content

            * Avoid using fopen() for RAII/OOD reasons. If necessary,
              use _wfopen() and WinAPI conventions above.

          
    */

    // For interface to win32 API functions
    std::string convert(const std::wstring& str, unsigned int codePage = 0/*= CP_UTF8*/);
    std::wstring convert(const std::string& str, unsigned int codePage = 0/*= CP_UTF8*/);
    std::string convert(const wchar_t* str, unsigned int codePage = 0/*= CP_UTF8*/);
    std::wstring convert(const char* str, unsigned int codePage = 0/*= CP_UTF8*/);

    /*
    // Interface to MFC
    std::string convert(const CString& mfcString);
    CString convert(const std::string& s);
    */

    std::string strerror(int err_code);

#ifdef VT_USE_COM
    std::string com_error(long hresult);
#endif

    namespace Utils
    {
        template <typename T>
        bool insert_if_not_present(std::vector<T>& container, T value)
        {
            if ( std::find(container.begin(), container.end(), value ) == container.end() )
            {
                container.push_back(value);
                return true;
            }
            return false;
        }
    }
}


