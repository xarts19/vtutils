#pragma once

#include <string>
#include <stdint.h>

namespace VT
{

class GlobalVar
{
public:
    GlobalVar(const std::wstring& name);
    ~GlobalVar();

    /*
    */
    bool is_set();

    /*
    */
    bool reference();

    /*
    */
    bool dereference();

    /*
    */
    void lock();

    /*
    */
    void unlock();

private:
    bool OperateMapping( int32_t& count, int32_t mod_val );

private:
    std::wstring _name;

    struct Impl;
    Impl* _pImpl;
};


class GlobalVarLocker
{
public:
    GlobalVarLocker(GlobalVar& gv)
        : gv_(gv)
    {
        gv_.lock();
    }

    ~GlobalVarLocker()
    {
        gv_.unlock();
    }

private:
    GlobalVarLocker(const GlobalVarLocker&);
    GlobalVarLocker& operator=(const GlobalVarLocker&);

    GlobalVar& gv_;
};

}
