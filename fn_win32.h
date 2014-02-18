
/**

<!-- Copyright (c) Rocket Division Software 2003-2008. All rights reserved. -->
<!-- Copyright (c) ArCycle Software LTD, 2011. All rights reserved. -->

@file   fn_win32.h

@brief      Win32 wrappers.

    Cs          - CRITICAL_SECTION
    Event       - event
    Handle      - HANDLE
    Mutex       - mutex
    Semaphore   - semaphore

@author Alexey Pakhunov

@date   $Id: fn_win32.h 2214 2010-12-29 17:51:54Z dance $

	2003-11-30 PAA : Created.
*/

#ifndef __FN_WIN32_H_INCLUDED
#define __FN_WIN32_H_INCLUDED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include "fn_common.h"
//#include "Log.h"

//
// Class:     Handle
//
// Purpose:   A wrapper for Win32 HANDLE.
//

class Handle
{
public:
    //
    // Construction/destruction
    //

    Handle(): m_h(0) {}

    Handle(
        IN HANDLE h
        )
    :
        m_h(h)
    {}

    Handle(
        IN const Handle& h
        )
    :
        m_h(0)
    {
        operator=(h);
    }

    ~Handle()
    {
        close();
    }

    HANDLE
    handle() const
    {
        return m_h;
    }

    //
    // Function:  isOk
    //
    // Purpose:   Check whether handle is valid
    //

    bool
    isOk() const
    {
        return (m_h && m_h != INVALID_HANDLE_VALUE);
    }

    //
    // Function:  attach
    //
    // Purpose:   Attaches Win32 'HANDLE' to 'Handle'.
    //

    void
    attach(
        IN HANDLE h
        )
    {
		HANDLE old = InterlockedExchangePointer( &m_h, h );

		if( old && old != INVALID_HANDLE_VALUE )
		{
			CloseHandle( old );
		}
    }

    //
    // Function:  detach
    //
    // Purpose:   Detaches Win32 'HANDLE' from 'Handle'.
    //

    HANDLE
    detach()
    {
        return InterlockedExchangePointer( &m_h, 0 );
    }

    //
    // Function:  close
    //
    // Purpose:   Closes controlled handle.
    //

    void
    close()
    {
		HANDLE h = InterlockedExchangePointer( &m_h, 0 );

		if( h && h != INVALID_HANDLE_VALUE )
		{
			CloseHandle( h );
        }
    }

    //
    // Function:  operator=
    //
    // Purpose:   Duplicates handle controlled by given 'Handle' object
    //

    Handle&
    operator=(
        IN const Handle& h
        )
    {
        HANDLE handle = 0;

        BOOL result =
            DuplicateHandle(
                GetCurrentProcess(),
                h.m_h,
                GetCurrentProcess(),
                &handle,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS);

        if (!result)
        {
            //LOG_ERROR(F_DBG, ("'DuplicateHandle' failed: error %d.", GetLastError()));
        }

        close();
        m_h = handle;

        return *this;
    }

private:
    HANDLE m_h;
};


//
// Class:     Event
//
// Purpose:   A wrapper for Win32 event.
//

class Event: public Handle
{
public:
    Event(
        IN BOOL manualReset = true,
        IN BOOL initialState = false
        )
    {
        attach(CreateEvent(NULL, manualReset, initialState, NULL));
    }

    void
    set()
    {
        SetEvent(handle());
    }

    void
    reset()
    {
        ResetEvent(handle());
    }

    DWORD
    isSet()
    {
        return WaitForSingleObject( handle(), 0 ) == WAIT_OBJECT_0;
    }
};


//
// Class:     Mutex
//
// Purpose:   A wrapper for Win32 mutex.
//

class Mutex: public Handle
{
public:

    class Owner;

    Mutex()
    {}

    Mutex(
        IN BOOL initialOwner
        )
    {
        attach(CreateMutex(NULL, initialOwner, NULL));
    }

    void
    release()
    {
        ReleaseMutex(handle());
    }
};


////////////////

#if 1

//
// Class:     Cs
//
// Purpose:   A wrapper for Win32 CRITICAL_SECTION.
//

class Cs
{
public:

    class Owner;

    //
    // Critical secition interface
    //

    Cs()
    {
        //InitializeCriticalSection(&m_cs);

		//
		// Windows 2000/XP: If the high-order bit is set, the function preallocates
		// the event used by the EnterCriticalSection function.
		//
		InitializeCriticalSectionAndSpinCount( &m_cs, 0x80001000 );

		m_counter = 0;
    }

    ~Cs()
    {
		//if( m_counter )
		//	LOG_ERROR(21, ("Destructor with active %p (%d)", this, m_counter));

		while( InterlockedDecrement( &m_counter ) >= 0 )
		{
			LeaveCriticalSection(&m_cs);
		}

        DeleteCriticalSection( &m_cs );
    }

    void
    lock()
    {
		//LOG_DEBUG(21, (">>> Lock this = 0x%p", this));

        EnterCriticalSection(&m_cs);

		InterlockedIncrement( &m_counter );

		//LOG_DEBUG(21, ("<<< Lock this = 0x%p, m_counter = %d", this, m_counter));
    }

	BOOL
	try_lock()
	{
		//LOG_DEBUG(21, (">>> Lock this = 0x%p", this));
		BOOL bResult = FALSE;

		bResult = TryEnterCriticalSection(&m_cs);
		
		if (bResult)
		{
			InterlockedIncrement( &m_counter );
		}		

		//LOG_DEBUG(21, ("<<< Lock this = 0x%p, m_counter = %d", this, m_counter));

		return bResult;
	}

    void
    unlock()
    {
		//LOG_DEBUG(21, (">>> Unlock this = 0x%p", this));

		InterlockedDecrement( &m_counter );

        LeaveCriticalSection(&m_cs);

		//LOG_DEBUG(21, ("<<< Unlock this = 0x%p, m_counter = %d", this, m_counter));
    }

private:

    //
    // Disable coping
    //

    Cs(const Cs&);
    Cs& operator=(const Cs&);

private:
    CRITICAL_SECTION m_cs;
	LONG	m_counter;
};


#else

//
// Class:     Cs
//
// Purpose:   Critical section built on top of Event object. There is only 
//            difference with standard CS - waiting on event implemented as 
//            a poll cycle with 100ms intervals. This is a workaround for 
//            the problem with hanging simultaneous Enter/LeaveCriticalSection
//            on HyperThreading coputer running clean WinXP Pro. This is 
//            not confirmed bug though there is one computer where it is 
//            reproducable.
//

class Cs
{
public:

    class Owner;

    //
    // Critical secition interface
    //

    Cs(): 
        m_event(FALSE, FALSE),
        m_count(-1),
        m_recursion(0),
        m_owner(0)
    {}

    void
    lock()
    {
        if (InterlockedIncrement(&m_count))
        {
            if (m_owner == ::GetCurrentThreadId())
            {
                ++m_recursion;
                return;
            }

            while (WAIT_TIMEOUT == WaitForSingleObject(m_event.handle(), 100));
        }

        m_owner = ::GetCurrentThreadId();
        m_recursion = 1;
    }

    void
    unlock()
    {
		assert(m_owner == ::GetCurrentThreadId());

        if (--m_recursion)
        {
            InterlockedDecrement(&m_count);
        }
        else
        {
            m_owner = 0;
            if (InterlockedDecrement(&m_count) >= 0)
            {
                m_event.set();
            }
        }
    }

private:

    //
    // Disable coping
    //

    Cs(const Cs&);
    Cs& operator=(const Cs&);

private:
    Event m_event;
    LONG m_count;
    LONG m_recursion;
    DWORD m_owner;
};

#endif

//
// Class:     Cs::Owner
//
// Purpose:   Acquires a critical section object on contruction and releases
//            it on destruction.
//

class Cs::Owner
{
public:
    Owner(Cs* cs): m_cs(cs)
    {		
        m_cs->lock();		
    }

    Owner(Cs& cs): m_cs(&cs)
    {		
        m_cs->lock();	
    }

    ~Owner()
    {		
        m_cs->unlock();		
    }

private:
    Cs* m_cs;
};


//
// Class:     Semaphore
//
// Purpose:   A wrapper for Win32 semaphore.
//

class Semaphore: public Handle
{
public:
    Semaphore()
    {}

    Semaphore(
        IN LONG initialCount,
        IN LONG maximumCount
        )
    {
        attach(CreateSemaphore(NULL, initialCount, maximumCount, NULL));
    }

    void
    release(
        IN LONG count = 1
        )
    {
        LONG prev = 0;

		if( isOk() )
		{
			ReleaseSemaphore(handle(), count, &prev);
		}
    }
};


//
// Class:     WaitableTimer
//
// Purpose:   A wrapper for Win32 waitable timer.
//

class WaitableTimer: public Handle
{
public:
    WaitableTimer()
    {}

    WaitableTimer(
        IN BOOL manualReset
        )
    {
        attach(CreateWaitableTimer(NULL, manualReset, NULL));
    }

    void
    set(
        IN const LARGE_INTEGER* dueTime,
        IN LONG period = 0,
        IN BOOL resume = FALSE
        )
    {
        SetWaitableTimer(handle(), dueTime, period, NULL, NULL, resume);
    }

    void
    cancel()
    {
        CancelWaitableTimer(handle());
    }
};


//
// Class:     StopGuard
//
// Purpose:   Used for such scenarios like waiting for all occupied resources
//            to be released. StopGuard is in signalled state if 'count()' is equal
//            or greater than zero. 'acquire' and 'release' is used for decreasing
//            and increasing the counter.
//

class StopGuard
{
public:
    StopGuard(): m_count(0) {}

    StopGuard(
        IN LONG initial
        )
    :
        m_count(0),
        m_event(TRUE, FALSE)
    {
        release(initial);
    }

    void
    acquire(LONG delta = 1)
    {
        release(-delta);
    }

    void
    release(LONG delta = 1)
    {
        m_cs.lock();

        m_count += delta;
        if (count() < 0)
            m_event.reset();
        else
            m_event.set();

        m_cs.unlock();
    }

    LONG
    count() const
    {
        return m_count;
    }

    HANDLE
    handle() const
    {
        return m_event.handle();
    }

private:
    Cs m_cs;
    LONG m_count;
    Event m_event;
};


//
// Class:     Thread
//
// Purpose:   A wrapper for HANDLE of a thread. Besides of thread handle keeps thread id .
//

class Thread: public Handle
{
public:
    Thread(): 
        m_id(0xffffffffu)
    {}

    Thread(
        LPSECURITY_ATTRIBUTES   lpThreadAttributes,
        SIZE_T                  dwStackSize,
        LPTHREAD_START_ROUTINE  lpStartAddress,
        LPVOID                  lpParameter,
        DWORD                   dwCreationFlags
        )
    {
        attach(CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, &m_id));
    }

    Thread(
        IN const Thread& t
        )
    :
        Handle(static_cast<const Handle&>(t))
    {
        m_id = t.id();
    }

    Thread&
    operator=(
        IN const Thread& t
        )
    {
        Handle::operator=(static_cast<const Handle&>(t));
        m_id = t.id();

        return *this;
    }

    DWORD
    id() const
    {
        return m_id;
    }

private:
    DWORD m_id;
};


#endif // __FN_WIN32_H_INCLUDED
