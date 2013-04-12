#include "VTDynLib.h"

#include <stdexcept>
#include <dlfcn.h>
#include <assert.h>
#include <stddef.h>


struct VT::DynLib::Impl
{
    Impl()
        : handle(NULL)
    { }
    
    void* handle;
};


VT::DynLib::DynLib(const char* name)
    : pimpl_(new Impl)
{
    pimpl_->handle = dlopen(name, RTLD_NOW);
    if (pimpl_->handle == NULL)
        throw std::runtime_error(dlerror());
}


VT::DynLib::~DynLib()
{
    assert(pimpl_->handle);
    dlclose(pimpl_->handle);
    delete pimpl_;
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
    return get_symbol(pimpl_->handle, name);
}


VT::DynLib::fun_ptr VT::DynLib::get_fun_symbol(const char* name)
{
    void* ptr = get_symbol(pimpl_->handle, name);
    
    // preventing warnings for casting data pointer to function pointer
    // we need to do this because of ancient POSIX API for dlsym
    return reinterpret_cast<fun_ptr>(reinterpret_cast<size_t>(ptr));
}
