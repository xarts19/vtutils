#pragma once

namespace VT
{
    class DynLib
    {
    public:
        typedef int (*fun_ptr)();
        typedef void* data_ptr;
        
        DynLib(const char* name);
        ~DynLib();
        
        data_ptr get_data_symbol(const char* name);
        fun_ptr get_fun_symbol(const char* name);
        
    private:
        struct Impl;
        Impl* pimpl_;
    };
}
