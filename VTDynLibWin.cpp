#include "VTDynLib.h"

#include "VTUtil.h"

#include <Windows.h>

#include <stdexcept>
#include <assert.h>


struct VT::DynLib::Impl
{
    Impl()
        : handle(NULL)
    { }

    HMODULE handle;
};


VT::DynLib::DynLib(const char* name)
    : pimpl_(new Impl)
{
    pimpl_->handle = LoadLibraryA(name);
    if (pimpl_->handle == NULL)
        throw std::runtime_error(VT::strerror(GetLastError()));
}


VT::DynLib::~DynLib()
{
    assert(pimpl_->handle);
    FreeLibrary(pimpl_->handle);
    delete pimpl_;
}


namespace
{
    FARPROC get_symbol(HMODULE handle, const char* name)
    {
        assert(handle);
        FARPROC ptr = GetProcAddress(handle, name);
        if (ptr == NULL)
            throw std::runtime_error(VT::strerror(GetLastError()));
        return ptr;
    }
}


VT::DynLib::data_ptr VT::DynLib::get_data_symbol(const char* name)
{
    FARPROC ptr = get_symbol(pimpl_->handle, name);
    return reinterpret_cast<void*>(ptr);
}


VT::DynLib::fun_ptr VT::DynLib::get_fun_symbol(const char* name)
{
    return reinterpret_cast<fun_ptr>(get_symbol(pimpl_->handle, name));
}
