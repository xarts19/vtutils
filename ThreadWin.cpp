#include <Windows.h>

#include "Thread.h"

typedef DWORD (*THREAD_START_WIN)(void* lpParameter);

const unsigned int Thread::T_INFINITE = INFINITE;

class ThreadWin : public Thread
{
public:

	ThreadWin(HANDLE thread): _thread(thread) {}
	~ThreadWin() {}

	void join(unsigned int timeout)
	{
		WaitForSingleObject(_thread, timeout);
	}

private:
	HANDLE _thread;
};

Thread* Thread::create(	size_t stack_size,
						THREAD_START start_address,
						void* parameter,
						unsigned int creation_flags,
						unsigned int* thread_id)
{
	HANDLE thread = CreateThread(NULL, stack_size, reinterpret_cast<LPTHREAD_START_ROUTINE>(start_address), parameter, creation_flags, (LPDWORD)thread_id);
	if(!thread)
	{
		return NULL;
	}
	else
	{
		Thread* t = new ThreadWin(thread);
		return t;
	}
}

void Thread::Sleep(unsigned int time_millisecond)
{
	::Sleep(time_millisecond);
}
