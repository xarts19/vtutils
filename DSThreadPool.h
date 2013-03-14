/*
    Author: Anton Akhmetshin
*/
#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <list>
#include <vector>
#include <memory>

typedef void (*fnThdWorker)(PVOID pContext);
struct ThreadData
{
    HANDLE hThread;
    HANDLE hWaitHandle;
    HANDLE hFreeHandle;
    PVOID  lpContext;
    bool   terminateFlag;
    fnThdWorker pCallback;

    ThreadData()
        : hThread(NULL)
        , lpContext(nullptr)
        , terminateFlag(false)
        , pCallback(nullptr)
        , hWaitHandle(NULL)
        , hFreeHandle(NULL)
    {
    }

    ~ThreadData()
    {
        CloseHandle(hWaitHandle);
        CloseHandle(hFreeHandle);
    }
};

class CDSThreadPool
{
public:
    CDSThreadPool(void);
    CDSThreadPool(size_t thdCount);
    ~CDSThreadPool(void);

    /*
    */
    bool SetWokerNum(size_t thdCount);

    /*
    */
    bool QueueWork(fnThdWorker pWorker, LPVOID lpContext);

    /*
    */
    bool WaitForCompletion();

private:

    /*
    */
    ThreadData* GetFirstFreeThread(bool waitForOne = false);

    /*
    */
    int GetFreeThreads( std::vector<ThreadData*>& thds );

    /*
    */
    static
    DWORD CALLBACK WorkerThd(PVOID lpParam);

    /*
    */
    static
    DWORD CALLBACK SchedulerThd(PVOID lpParam);

private:
    std::list<ThreadData> m_threads;
    std::list<std::pair<fnThdWorker, LPVOID>> m_workQueue;
    std::vector<HANDLE> m_waitHandles;
    std::vector<ThreadData*> m_thdArray;

    HANDLE m_hQueueEvent;
    HANDLE m_hSchedThd;
    bool m_active;
};

