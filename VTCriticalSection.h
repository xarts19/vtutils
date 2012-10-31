#pragma once

namespace VT
{

    class CriticalSection
    {

    public:
        CriticalSection();
        ~CriticalSection();

        void enter();
        void leave();

    private:
        struct critical_section_data;
        critical_section_data* data;
    };

};