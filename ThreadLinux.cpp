#include <unistd.h>
#include <limits.h>
#include <pthread.h>

#include "Thread.h"

typedef void* (*THREAD_START_LINUX)(void* lpParameter);

const unsigned int Thread::T_INFINITE = UINT_MAX;

class ThreadLinux : public Thread
{
public:

	ThreadLinux(pthread_t thread): _thread(thread) {}
	~ThreadLinux() {}

	void join(unsigned int timeout)
	{
		pthread_join(_thread, NULL);
	}

private:
	pthread_t _thread;
};

Thread* Thread::create(	size_t stack_size,
						THREAD_START start_address,
						void* parameter,
						unsigned int creation_flags,
						unsigned int* thread_id)
{
	pthread_t thread;
	int fail = pthread_create(&thread, NULL, (THREAD_START_LINUX)start_address, parameter);
	if(fail)
	{
		return NULL;
	}
	else
	{
		Thread* t = new ThreadLinux(thread);
		return t;
	}
}

void Thread::Sleep(unsigned int time_millisecond)
{
	usleep(time_millisecond * 1000);
}
