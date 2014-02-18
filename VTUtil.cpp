#include "VTUtil.h"

#include "VTEncodeConvert.hpp"

#include <string>
#include <random>
#include <limits>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <stdexcept>

std::string VT::human_readable_size(unsigned long long size, int precision)
{
    double dsize = static_cast<double>(size);
    unsigned int unit = 0;
    const char* units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    while (dsize > 1024)
    {
        dsize /= 1024;
        ++unit;
    }
    
    assert(unit < sizeof(units) / sizeof(units[0]));

    if (precision == -1)
    {
        int i = 0;
        double delta = (dsize - int(dsize)) * 10;
        while (i < 5 && delta > 3)
        {
            delta = (delta - int(delta)) * 10;
            ++i;
        }
        precision = i;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << dsize << " " << units[unit];
    return ss.str();
}


time_t VT::parse_datetime(const std::string& str)
{
    unsigned int yy = 0, mm = 0, dd = 0, h = 0, m = 0, s = 0;

    int num_read = sscanf(str.c_str(), "%u.%u.%u %u:%u:%u", &yy, &mm, &dd, &h, &m, &s);
    if (num_read != 5 && num_read != 6)
        throw std::runtime_error("Wrong date-time string");

    if (yy < 1900)
        throw std::runtime_error("Years below 1900 are not supported");

    if (!(1 <= mm && mm <= 12))
        throw std::runtime_error("Wrong month value");

    if (!(1 <= dd && dd <= 31))
        throw std::runtime_error("Wrong day value");

    if (!(h <= 23))
        throw std::runtime_error("Wrong hours value");

    if (!(m <= 59))
        throw std::runtime_error("Wrong minutes value");

    if (!(s <= 59))
        throw std::runtime_error("Wrong seconds value");

    struct tm c_tm;
    memset(&c_tm, 0, sizeof(c_tm));
    c_tm.tm_isdst = -1;        // to avoid problems with DST
    c_tm.tm_year = yy - 1900;  // see tm specification
    c_tm.tm_mon = mm - 1;      // see tm specification
    c_tm.tm_mday = dd;
    c_tm.tm_hour = h;
    c_tm.tm_min = m;
    c_tm.tm_sec = s;

    time_t res = mktime(&c_tm);
    if (res == -1)
        throw std::runtime_error("Failed to convert tm to time_t");

    return res;
}


void VT::create_system_exception(const std::string& what, int error_code, const std::string& error_msg)
{
    if (error_code == 0)
        error_code = VT::last_error();

    std::stringstream ss;
    ss << what << ", code=" << error_code
       << " [0x" << std::hex << std::setw(8);
    auto ch = ss.fill('0');
    ss << error_code  << "] (";
    ss.fill(ch);
    ss << (error_msg == "" ? VT::strerror(error_code) : error_msg) << ")";

    throw std::runtime_error(ss.str());
}

std::string VT::time_as_string(time_t t, const char* format)
{
    struct tm* timeInfo;
    const size_t size = 256;
    char buffer[size];
    timeInfo = localtime(&t);
    strftime(buffer, size, format, timeInfo);
    return buffer;
}


std::wstring VT::time_as_wstring(time_t t, const char* format)
{
    return VT::UTF8ToWstring(time_as_string(t, format));
}


uint64_t VT::GenerateGUID()
{
    std::random_device rd;
    static std::mt19937_64 generator(rd());
    static std::uniform_int_distribution<uint64_t> distribution(1, (std::numeric_limits<uint64_t>::max)());

    return distribution(generator);
}


std::string VT::str_from_cstr(const char* cstr, std::size_t size)
{
    return std::string(cstr, strnlen(cstr, size));
}


std::size_t VT::safe_strlen(const char* str, std::size_t max)
{
    const void* res = std::memchr(str, '\0', max);
    if (!res)
        return max;
    else
        return static_cast<const char*>(res) - str;
}


void VT::copy_str(const char* src, size_t src_len, char* dest, size_t dest_size)
{
    if (src_len > dest_size)
        throw std::runtime_error("copy_str buffer overflow");

    memcpy(dest, src, src_len);
}


void VT::copy_str(const std::string& src, char* dest, size_t size)
{
    copy_str(src.c_str(), src.size() + 1, dest, size);  // + 1 is to copy '\0' (std::string::size() doesn't include '\0')
}
