#pragma once

typedef unsigned int (*THREAD_START)(void* lpParameter);

class Thread
{
public:
	static const unsigned int T_INFINITE;

	static Thread* create(	size_t stack_size,
							THREAD_START start_address,
							void* parameter,
							unsigned int creation_flags,
							unsigned int* thread_id);
	virtual ~Thread() {}

	virtual void join(unsigned int timeout) = 0;

	static void Sleep(unsigned int time_millisecond);
};
