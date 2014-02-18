#include "VTStringUtil.h"
#include "VTUtil.h"

#include <cctype>
#include <algorithm>

namespace detail_
{
    char char_to_lower(char in)
    {
        return static_cast<char>( std::tolower(in) );
    }
}

std::string VT::to_lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), detail_::char_to_lower);
    return str;
}

template <typename Ch>
bool starts_with_helper(const std::basic_string<Ch>& str, const std::basic_string<Ch>& prefix)
{
    return 0 == str.compare(0, prefix.size(), prefix);
}

template <typename Ch>
bool ends_with_helper(const std::basic_string<Ch>& str, const std::basic_string<Ch>& suffix)
{
    if (str.size() < suffix.size())
        return false;

    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}


bool VT::starts_with(const std::string& str, const std::string& prefix)
{
    return starts_with_helper(str, prefix);
}


bool VT::starts_with(const std::wstring& str, const std::wstring& prefix)
{
    return starts_with_helper(str, prefix);
}


bool VT::ends_with(const std::string& str, const std::string& suffix)
{
    return ends_with_helper(str, suffix);
}


bool VT::ends_with(const std::wstring& str, const std::wstring& suffix)
{
    return ends_with_helper(str, suffix);
}
