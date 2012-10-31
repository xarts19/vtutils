#include "VTCriticalSection.h"
#include <pthread.h>

struct VTCriticalSection::critical_section_data
{
    pthread_mutex_t _criticalSection;
};

VTCriticalSection::VTCriticalSection()
{
    data = new critical_section_data();
    pthread_mutex_init( &data->_criticalSection, NULL );
}

VTCriticalSection::~VTCriticalSection()
{
    pthread_mutex_destroy( &data->_criticalSection );
    delete data;
}

void VTCriticalSection::enter()
{
    pthread_mutex_lock( &data->_criticalSection );
}

void VTCriticalSection::leave()
{
    pthread_mutex_unlock( &data->_criticalSection );
}
