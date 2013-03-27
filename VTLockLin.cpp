#include "VTLock.h"

#include <pthread.h>

struct VT::Lock::Impl
{
    pthread_mutex_t _criticalSection;
};

VT::Lock::Lock() : data( new Impl() )
{
    pthread_mutex_init( &data->_criticalSection, NULL );
}

VT::Lock::~Lock()
{
    pthread_mutex_destroy( &data->_criticalSection );
    delete data;
}

void VT::Lock::enter()
{
    pthread_mutex_lock( &data->_criticalSection );
}

bool VT::Lock::try_enter()
{
    return pthread_mutex_trylock( &data->_criticalSection ) == 0;
}

void VT::Lock::leave()
{
    pthread_mutex_unlock( &data->_criticalSection );
}
