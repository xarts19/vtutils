#pragma once

namespace VT
{
    class DynLib
    {
    public:
        typedef int (*fun_ptr)();
        typedef void* data_ptr;
        
        DynLib();  // create empty DynLib that doesn't manage anything
        explicit DynLib(const wchar_t* name, bool expand_name = false, bool prepend_exec_path = false);
        ~DynLib();

        DynLib(DynLib&& other);
        DynLib& operator=(DynLib&& other);
        
        data_ptr get_data_symbol(const char* name);
        fun_ptr get_fun_symbol(const char* name);

        operator bool();
        
    private:
        DynLib(const DynLib&);
        DynLib& operator=(const DynLib&);

        struct Impl;
        Impl* pimpl_;
    };
}
