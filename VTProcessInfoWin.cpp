#include "VTProcessInfo.h"

#include "windows.h"
#include "psapi.h"

VT::ProcessInfo VT::get_process_info()
{
    ProcessInfo info;

    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    info.used_virtual_memory = pmc.PagefileUsage;
    info.used_physical_memory = pmc.WorkingSetSize;
    info.id = GetCurrentProcessId();

    return info;
}

VT::SystemInfo VT::get_system_info()
{
    SystemInfo info;

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    // Note: The name "TotalPageFile" is a bit misleading here.
    // In reality this parameter gives the "Virtual Memory Size",
    // which is size of swap file plus installed RAM.
    info.total_virtual_memory = memInfo.ullTotalPageFile;
    info.available_virtual_memory = memInfo.ullAvailPageFile;
    info.used_virtual_memory = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

    info.total_physical_memory = memInfo.ullTotalPhys;
    info.available_physical_memory = memInfo.ullAvailPhys;
    info.used_physical_memory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

    return info;
}

