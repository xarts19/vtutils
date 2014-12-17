#pragma once

#include <cstdint>

namespace VT
{
    class SystemInfo
    {
    public:
        uint64_t total_virtual_memory;  // Physical + Swap
        uint64_t used_virtual_memory;
        uint64_t available_virtual_memory;

        uint64_t total_physical_memory;
        uint64_t used_physical_memory;
        uint64_t available_physical_memory;
    };

    class ProcessInfo
    {
    public:
        uint32_t id;
        uint64_t used_virtual_memory;
        uint64_t used_physical_memory;
    };

    ProcessInfo get_process_info();
    SystemInfo get_system_info();
}
