/*
    Author: Anton Akhmetshin
*/
#include "DSThreadPool.h"


CDSThreadPool::CDSThreadPool(void)
    : m_active(true)
{
    m_hQueueEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hSchedThd   = ::CreateThread(NULL, 0, &CDSThreadPool::SchedulerThd, this, 0, NULL);
}

CDSThreadPool::CDSThreadPool( size_t thdCount )
    : CDSThreadPool()
{
    SetWokerNum(thdCount);
}

CDSThreadPool::~CDSThreadPool(void)
{
    // Close scheduler
    m_active = false;
    ::SetEvent(m_hQueueEvent);
    ::WaitForSingleObject(m_hSchedThd, INFINITE);

    SetWokerNum(0);
    ::CloseHandle(m_hQueueEvent);
}

bool CDSThreadPool::SetWokerNum( size_t thdCount )
{
    // Create new threads
    while(thdCount > m_threads.size())
    {
        m_threads.emplace_back(ThreadData());
        auto& data = m_threads.back();

        data.hFreeHandle = ::CreateEvent(NULL, TRUE, TRUE,  NULL);
        data.hWaitHandle = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        data.hThread     = ::CreateThread(NULL, 0, &CDSThreadPool::WorkerThd, &data, 0, NULL);

        m_thdArray.emplace_back(&data);
        m_waitHandles.emplace_back(data.hFreeHandle);
    }

    if(thdCount == m_threads.size())
        return true;

    // Trim existing threads
    while(thdCount < m_threads.size())
    {
        auto& data = m_threads.back();

        data.terminateFlag = true;

        ::SetEvent(data.hWaitHandle);
        ::WaitForSingleObject(data.hThread, INFINITE);

        m_threads.pop_back();
        m_thdArray.erase(m_thdArray.end() - 1);
        m_waitHandles.erase(m_waitHandles.end() - 1);
    }

    if(thdCount == m_threads.size())
        return true;

    return false;
}

bool CDSThreadPool::QueueWork( fnThdWorker pWorker, LPVOID lpContext )
{
    // Execute in free thread
    if(ThreadData *pThread = GetFirstFreeThread())
    {
        pThread->pCallback = pWorker;
        pThread->lpContext = lpContext;

        // Render thread busy
        ::ResetEvent(pThread->hFreeHandle);

        // Wake up thread
        ::SetEvent(pThread->hWaitHandle);

        return true;
    }
    // Queue execution
    else
    {
        m_workQueue.emplace_back(std::make_pair(pWorker, lpContext));

        ::SetEvent(m_hQueueEvent);

        return false;
    }
}


bool CDSThreadPool::WaitForCompletion()
{
    int index = 0;

    // Wait for queue
    while(m_workQueue.size() > 0)
        WaitForSingleObject(m_hQueueEvent, 0);

    // Wait for worker threads
    WaitForMultipleObjects((DWORD)m_waitHandles.size(), m_waitHandles.data(), TRUE, INFINITE);

    return true;
}

ThreadData* CDSThreadPool::GetFirstFreeThread( bool waitForOne /*= false*/ )
{
    DWORD index = 0;
    ThreadData *retval = nullptr;

    index = WaitForMultipleObjects((DWORD)m_waitHandles.size(), m_waitHandles.data(), FALSE, waitForOne ? INFINITE : 0) - WAIT_OBJECT_0;

    if(index != 0xffffffff && index < m_threads.size())
        retval = m_thdArray[index];

    return retval;
}

int CDSThreadPool::GetFreeThreads( std::vector<ThreadData*>& thds )
{
    for(auto& item : m_threads)
    {
        if( WaitForSingleObject(item.hFreeHandle, 1) == ERROR_SUCCESS )
            thds.emplace_back(&item);
    }

    return (int)thds.size();
}

////////////////////////////////////////////////

DWORD CALLBACK CDSThreadPool::SchedulerThd( PVOID lpParam )
{
    CDSThreadPool *pInstance = (CDSThreadPool*)lpParam;

    if(!pInstance)
        return -1;

    while (pInstance->m_active)
    {
        ::WaitForSingleObject(pInstance->m_hQueueEvent, INFINITE);

        if(!pInstance->m_active)
            return 0;

        if(pInstance->m_workQueue.size() > 0)
        {
            auto& item = pInstance->m_workQueue.front();

            ThreadData* pThd = pInstance->GetFirstFreeThread(true);

            if(pThd != nullptr)
            {
                pThd->pCallback = item.first;
                pThd->lpContext = item.second;

                // Render thread busy
                ::ResetEvent(pThd->hFreeHandle);

                // Wake worker thread
                ::SetEvent(pThd->hWaitHandle);

                pInstance->m_workQueue.pop_front();
            } 
        }
        else
            ::ResetEvent(pInstance->m_hQueueEvent);
    }

    return 0;
}

DWORD CALLBACK CDSThreadPool::WorkerThd( PVOID lpParam )
{
    ThreadData* pData = (ThreadData*)lpParam;

    while (!pData->terminateFlag)
    {
        ::WaitForSingleObject(pData->hWaitHandle, INFINITE);

        if(pData->terminateFlag)
            break;

        // Execute work
        if(pData->pCallback)
        {
            pData->pCallback(pData->lpContext);

            pData->pCallback = nullptr;
            pData->lpContext = nullptr;
        }

        // Continue sleep
        ::ResetEvent(pData->hWaitHandle);

        // Signal for free thread
        ::SetEvent(pData->hFreeHandle);
    }

    return 0;
}
