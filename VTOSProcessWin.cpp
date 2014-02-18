#include "VTOSProcess.h"

#include <windows.h>

namespace VT
{

struct GlobalVar::Impl
{
    HANDLE hMapping;
    HANDLE sync_lock2;
};

GlobalVar::GlobalVar( const std::wstring& name )
    : _name(name)
    , _pImpl(new Impl)
{
    _pImpl->hMapping = NULL;

    wchar_t mtxName[255] = {0};
    swprintf(mtxName, 255, L"_mutexProcess0x%X_%s", GetCurrentProcessId(), _name.c_str());

    _pImpl->sync_lock2 = CreateMutex(NULL, FALSE, mtxName);
}

GlobalVar::~GlobalVar()
{
    CloseHandle(_pImpl->sync_lock2);
    delete _pImpl;
}

bool GlobalVar::OperateMapping( int32_t& count, int32_t mod_val )
{
    bool res = true;
    wchar_t mapName[255] = {0};
    swprintf(mapName, 255, L"_mapProcess0x%X_%s", GetCurrentProcessId(), _name.c_str());

    _pImpl->hMapping = OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mapName);

    if(_pImpl->hMapping == NULL)
    {
        // Check only, can return now
        if(mod_val == 0)
            return false;

        _pImpl->hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x1000, mapName);
    }

    if(_pImpl->hMapping != NULL)
    {
        void* pData = MapViewOfFile(_pImpl->hMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

        if(pData)
        {
            int32_t* val = reinterpret_cast<int32_t*>(pData);
            (*val) += mod_val;
            count = *val;

            UnmapViewOfFile(pData);

            if(count <= 0)
            {
                CloseHandle(_pImpl->hMapping);
                _pImpl->hMapping = NULL;
            }
        }
        else
            res = false;
    }
    else
        res = false;


    return res;
}

bool GlobalVar::is_set()
{
    int32_t count = 0;

    if(OperateMapping(count, 0) && count > 0)
        return true;
    else
        return false;
}

bool GlobalVar::reference()
{
    int32_t count = 0;

    if(OperateMapping(count, 1) && count > 0)
        return true;
    else
        return false;
}

bool GlobalVar::dereference()
{
    int32_t count = 0;

    if(OperateMapping(count, -1))
        return true;
    else
        return false;
}

void GlobalVar::lock()
{
    WaitForSingleObject(_pImpl->sync_lock2, INFINITE);
}

void GlobalVar::unlock()
{
    ReleaseMutex(_pImpl->sync_lock2);
}

}
