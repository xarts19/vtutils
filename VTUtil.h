#pragma once

#include <string>
#include <memory>
#include <time.h>
#include <cstdint>

#include "VTCompilerSpecific.h"

// No variadic templates before Visual Studio 2013
#if defined(COMPILER_GCC) || (defined(COMPILER_MSVC) && _MSC_VER >= 1800)
    #define VT_VARIADIC_TEMPLATES_SUPPORTED
#endif

#ifdef VT_VARIADIC_TEMPLATES_SUPPORTED
    #include <utility>
#endif

#define VT_DISABLE_COPY(Class) Class(const Class &); Class & operator= (const Class &)
#define VT_UNUSED(x) ((void)(x))

#define VT_TIME_ISO8601 "%Y-%m-%dT%H:%M:%S"
#define VT_TIME_DEFAULT "%Y.%m.%d %H:%M:%S"

// Compile-time static array size
extern "C++"
template <typename T, size_t N>
char (*RtlpNumberOf(T(&)[N]))[N];

#define ARRAY_SIZE(A) (sizeof(*RtlpNumberOf(A)))

// disable annoying MSVC 'conditional expression is a constant'
#ifdef _MSC_VER
    #pragma warning(disable : 4127)
#endif

namespace VT
{
    int last_error();
    std::string strerror(int err_code);

#ifdef VT_USE_COM
    std::string com_error(long hresult);
#endif

    std::wstring executable_path();

    void create_system_exception(const std::string& what, int error_code = 0, const std::string& error_msg = "");

    // if precision == -1 : use optimal
    std::string human_readable_size(unsigned long long size, int precision = -1);

    // can handle only VT_TIME_DEFAULT for now
    time_t parse_datetime(const std::string& str);

    std::wstring hostname();

    std::string time_as_string(time_t t = time(nullptr), const char* format = VT_TIME_DEFAULT);
    std::wstring time_as_wstring(time_t t = time(nullptr), const char* format = VT_TIME_DEFAULT);

    uint64_t GenerateGUID();

    template <typename T>
    struct default_delete
    {
        void operator()(T* t)
        {
            delete t;
        }
    };

    template <typename T>
    struct noop_delete
    {
        void operator()(T*)
        { }
    };

    // resulting std::string WILL NOT contain '\0' as [size()-1] character
    std::string str_from_cstr(const char* cstr, std::size_t size);

    // resulting std::string WILL NOT contain '\0' as [size()-1] character
    template <std::size_t Size>
    std::string str_from_cstr(const char (&cstr)[Size])
    {
        return str_from_cstr(cstr, Size);
    }


    // returns max if no '\0' in first max bytes, or the length of the string (without '\0') otherwise
    std::size_t safe_strlen(const char* str, std::size_t max);

    template <std::size_t Size>
    std::size_t safe_strlen(const char (&str)[Size])
    {
        return safe_strlen(str, Size);
    }


    void copy_str(const char* src, size_t src_len, char* dest, size_t dest_size);

    template <std::size_t Size>
    void copy_str(const char (&src)[Size], char* dest, size_t dest_size)
    {
        size_t len = safe_strlen(src, Size);
        copy_str(src, len, dest, dest_size);
    }

    template <std::size_t Size>
    void copy_str(const char* src, size_t src_len, char (&dest)[Size])
    {
        copy_str(src, src_len, dest, Size);
    }

    template <std::size_t SizeSrc, std::size_t SizeDest>
    void copy_str(const char (&src)[SizeSrc], char (&dest)[SizeDest])
    {
        copy_str(src, dest, SizeDest);
    }

    void copy_str(const std::string& src, char* dest, size_t size);

    template <std::size_t Size>
    void copy_str(const std::string& src, char (&dest)[Size])
    {
        copy_str(src, dest, Size);
    }


#ifdef VT_VARIADIC_TEMPLATES_SUPPORTED
    template <typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(std::forward<Args>(args)...);
    }

#else
    #define _MAKE_UNIQUE(TEMPLATE_LIST, PADDING_LIST, LIST, COMMA, X1, X2, X3, X4)	\
    \
    template<class T COMMA LIST(_CLASS_TYPE)>   \
    inline std::unique_ptr<T> make_unique(LIST(_TYPE_REFREF_ARG))   \
    {   \
        return std::unique_ptr<T>(new T(LIST(_FORWARD_ARG)));   \
    }

    _VARIADIC_EXPAND_0X(_MAKE_UNIQUE, , , , )
    #undef _MAKE_UNIQUE
#endif

}


