#include "VTCriticalSection.h"
#include <Windows.h>

struct VT::CriticalSection::Impl
{
    CRITICAL_SECTION criticalSection_;
};

VT::CriticalSection::CriticalSection() : data( new Impl() )
{
    InitializeCriticalSection( &data->criticalSection_ );
}

VT::CriticalSection::~CriticalSection()
{
    DeleteCriticalSection( &data->criticalSection_ );
    delete data;
}

void VT::CriticalSection::enter()
{
    EnterCriticalSection( &data->criticalSection_ );
}

bool VT::CriticalSection::try_enter()
{
    return TryEnterCriticalSection( &data->criticalSection_ ) == 0;
}

void VT::CriticalSection::leave()
{
    LeaveCriticalSection( &data->criticalSection_ );
}
