#include "VTDynLib.h"

#include "VTEncodeConvert.hpp"

#include "VTUtil.h"

#include <stdexcept>
#include <assert.h>
#include <stddef.h>

#include <dlfcn.h>

struct VT::DynLib::Impl
{
    Impl()
        : handle(NULL)
    { }
    
    void* handle;
};


VT::DynLib::DynLib()
    : pimpl_(nullptr)
{ }


VT::DynLib::DynLib(const wchar_t* name, bool expand_name, bool prepend_exec_path)
    : pimpl_(new Impl)
{
    std::string full_name = VT::WstringToUTF8(name);

    if (expand_name)
        full_name = "lib" + full_name + ".so";

    if (prepend_exec_path)
        full_name = VT::WstringToUTF8(VT::executable_path()) + full_name;

    pimpl_->handle = dlopen(full_name.c_str(), RTLD_NOW);

    if (pimpl_->handle == NULL)
        throw std::runtime_error(dlerror());
}


VT::DynLib::~DynLib()
{
    if (pimpl_)
    {
        dlclose(pimpl_->handle);
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
    void* get_symbol(void* handle, const char* name)
    {
        assert(handle);
        void* ptr = dlsym(handle, name);
        if (ptr == NULL)
            throw std::runtime_error(dlerror());
        return ptr;
    }
}


VT::DynLib::data_ptr VT::DynLib::get_data_symbol(const char* name)
{
    if (!pimpl_)
        throw std::runtime_error("No dynamic library loaded");

    return get_symbol(pimpl_->handle, name);
}


VT::DynLib::fun_ptr VT::DynLib::get_fun_symbol(const char* name)
{
    if (!pimpl_)
        throw std::runtime_error("No dynamic library loaded");

    void* ptr = get_symbol(pimpl_->handle, name);
    
    // preventing warnings for casting data pointer to function pointer
    // we need to do this because of ancient POSIX API for dlsym
    return reinterpret_cast<fun_ptr>(reinterpret_cast<size_t>(ptr));
}

VT::DynLib::operator bool()
{
    return (pimpl_ != nullptr && pimpl_->handle != NULL);
}

