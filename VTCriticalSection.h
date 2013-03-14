#pragma once

namespace VT
{

    // At least on Windows, it's safe to call enter on already owned lock
    class CriticalSection
    {

    public:
        CriticalSection();
        ~CriticalSection();

        void enter();
        bool try_enter();
        void leave();

    private: // non-copyable, non-movable
        CriticalSection(const CriticalSection&);
        CriticalSection& operator=(const CriticalSection&);

    private:
        struct Impl;
        Impl* data;
    };
    
    class CSLocker
    {
    public:
        explicit CSLocker(CriticalSection& cs) : cs_(cs) { cs_.enter(); }
        ~CSLocker() { cs_.leave(); }
        
    private:
        CSLocker(const CSLocker&);
        CSLocker& operator=(const CSLocker&);
    
        CriticalSection& cs_;
    };

}
