#include "VTCriticalSection.h"
#include <pthread.h>

struct VT::CriticalSection::critical_section_data
{
    pthread_mutex_t _criticalSection;
};

VT::CriticalSection::CriticalSection() : data( new critical_section_data() )
{
    pthread_mutex_init( &data->_criticalSection, NULL );
}

VT::CriticalSection::~CriticalSection()
{
    pthread_mutex_destroy( &data->_criticalSection );
    delete data;
}

void VT::CriticalSection::enter()
{
    pthread_mutex_lock( &data->_criticalSection );
}

bool VT::CriticalSection::try_enter()
{
    static_assert("Not implemented");
}

void VT::CriticalSection::leave()
{
    pthread_mutex_unlock( &data->_criticalSection );
}
