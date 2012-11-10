#include "VTCriticalSection.h"
#include <Windows.h>

struct VT::CriticalSection::critical_section_data
{
    CRITICAL_SECTION _criticalSection;
};

VT::CriticalSection::CriticalSection() : data( new critical_section_data() )
{
    InitializeCriticalSection( &data->_criticalSection );
}

VT::CriticalSection::~CriticalSection()
{
    DeleteCriticalSection( &data->_criticalSection );
    delete data;
}

void VT::CriticalSection::enter()
{
    EnterCriticalSection( &data->_criticalSection );
}

void VT::CriticalSection::leave()
{
    LeaveCriticalSection( &data->_criticalSection );
}
