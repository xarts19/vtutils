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


VT::DynLib::DynLib()
    : pimpl_(nullptr)
{ }


VT::DynLib::DynLib(const wchar_t* name, bool expand_name, bool prepend_exec_path)
    : pimpl_(new Impl)
{
    auto errmode = GetErrorMode();

    // Disable system error message boxes
    SetErrorMode(SEM_FAILCRITICALERRORS);

    std::wstring full_name = name;

    if (expand_name)
        full_name.append(L".dll");

    if (prepend_exec_path)
        full_name = VT::executable_path() + full_name;

    pimpl_->handle = LoadLibraryW(full_name.c_str());
    SetErrorMode(errmode);

    if (pimpl_->handle == NULL)
        throw std::runtime_error(VT::strerror(GetLastError()));
}


VT::DynLib::~DynLib()
{
    if (pimpl_)
    {
        FreeLibrary(pimpl_->handle);
        delete pimpl_;
    }
}


VT::DynLib::DynLib(DynLib&& other)
    : pimpl_(nullptr)
{
    std::swap(pimpl_, other.pimpl_);
}


VT::DynLib& VT::DynLib::operator=(DynLib&& other)
{
    std::swap(pimpl_, other.pimpl_);
    return *this;
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
    if (!pimpl_)
        throw std::runtime_error("No dynamic library loaded");

    FARPROC ptr = get_symbol(pimpl_->handle, name);
    return reinterpret_cast<void*>(ptr);
}


VT::DynLib::fun_ptr VT::DynLib::get_fun_symbol(const char* name)
{
    if (!pimpl_)
        throw std::runtime_error("No dynamic library loaded");

    return reinterpret_cast<fun_ptr>(get_symbol(pimpl_->handle, name));
}

VT::DynLib::operator bool()
{
    return (pimpl_ != nullptr && pimpl_->handle != NULL);
}

