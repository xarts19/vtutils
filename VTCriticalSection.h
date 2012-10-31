#pragma once

class VTCriticalSection
{

public:
    VTCriticalSection();
    ~VTCriticalSection();

    void enter();
    void leave();

private:
    struct critical_section_data;
    critical_section_data* data;
};

