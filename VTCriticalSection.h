#pragma once

namespace VT
{

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
        struct critical_section_data;
        critical_section_data* data;
    };

};