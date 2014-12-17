#include "ProcessInfo.h"

#include "sys/types.h"
#include "sys/sysinfo.h"
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>

uint64_t parseLine(char* line)
{
    while (*line < '0' || *line > '9')
        line++;
    return std::stoull(line);
}

uint64_t get_status_value(const std::string& name)
{
    FILE* file = fopen("/proc/self/status", "r");
    uint64_t result = 0;
    char line[128];

    while (fgets(line, 128, file) != NULL)
    {
        if ((line, name.c_str(), name.size()) == 0)
        {
            result = parseLine(line);
            break;
        }
    }

    fclose(file);
    return result;
}

VT::ProcessInfo VT::get_process_info()
{
    ProcessInfo info;

    info.used_virtual_memory = get_status_value("VmSize:");
    info.used_physical_memory = get_status_value("VmRSS:");
    info.id = getpid();

    return info;
}

VT::SystemInfo VT::get_system_info()
{
    SystemInfo info;

    struct sysinfo memInfo;
    sysinfo(&memInfo);
    info.total_virtual_memory = memInfo.totalram;
    //Add other values in next statement to avoid int overflow on right hand side...
    info.total_virtual_memory += memInfo.totalswap;
    info.total_virtual_memory *= memInfo.mem_unit;

    info.available_virtual_memory = memInfo.freeram;
    info.available_virtual_memory += memInfo.freeswap;
    info.available_virtual_memory *= memInfo.mem_unit;

    info.used_virtual_memory = info.total_virtual_memory;
    info.used_virtual_memory -= info.available_virtual_memory;

    info.total_physical_memory = memInfo.totalram;
    info.total_physical_memory *= memInfo.mem_unit;

    info.available_physical_memory = memInfo.freeram;
    info.available_physical_memory *= memInfo.mem_unit;

    info.used_physical_memory = info.total_physical_memory;
    info.used_physical_memory -= info.used_physical_memory;

    return info;
}
