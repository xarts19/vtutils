#include "VTCriticalSection.h"
#include <Windows.h>

struct VTCriticalSection::critical_section_data
{
    CRITICAL_SECTION _criticalSection;
};

VTCriticalSection::VTCriticalSection()
{
    data = new critical_section_data();
    InitializeCriticalSection( &data->_criticalSection );
}

VTCriticalSection::~VTCriticalSection()
{
    DeleteCriticalSection( &data->_criticalSection );
    delete data;
}

void VTCriticalSection::enter()
{
    EnterCriticalSection( &data->_criticalSection );
}

void VTCriticalSection::leave()
{
    LeaveCriticalSection( &data->_criticalSection );
}
