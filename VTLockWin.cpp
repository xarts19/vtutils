#include "VTLock.h"

#include <Windows.h>

struct VT::Lock::Impl
{
    CRITICAL_SECTION criticalSection_;
};

VT::Lock::Lock() : data( new Impl() )
{
    InitializeCriticalSection( &data->criticalSection_ );
}

VT::Lock::~Lock()
{
    DeleteCriticalSection( &data->criticalSection_ );
    delete data;
}

void VT::Lock::enter()
{
    EnterCriticalSection( &data->criticalSection_ );
}

bool VT::Lock::try_enter()
{
    return TryEnterCriticalSection( &data->criticalSection_ ) == 0;
}

void VT::Lock::leave()
{
    LeaveCriticalSection( &data->criticalSection_ );
}
