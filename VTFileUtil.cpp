#include "VTFileUtil.h"

#include "VTEncodeConvert.hpp"
#include "VTStringUtil.h"

#include <fstream>

template <typename Container>
bool read_helper(const std::wstring& path, Container& container)
{
    std::basic_ifstream<typename Container::value_type> f(VT_TO_FILENAME_ENCODING(path), std::ios_base::binary);

    if (!f.good())
        return false;

    //typename Container::value_type buffer[128];
    //f.rdbuf()->pubsetbuf(buffer, 128);

    container.assign((std::istreambuf_iterator<typename Container::value_type>(f)),
                      std::istreambuf_iterator<typename Container::value_type>());
    container.push_back(VT::ensure_tchar<typename Container::value_type>('\0'));

    return true;
}


bool VT::read_file(const std::wstring& path, std::string& buffer)
{
    return read_helper(path, buffer);
}


bool VT::read_file(const std::wstring& path, std::vector<char>& buffer)
{
    return read_helper(path, buffer);
}

bool VT::read_file(const std::wstring& path, std::wstring& buffer)
{
    return read_helper(path, buffer);
}


bool VT::read_file(const std::wstring& path, std::vector<wchar_t>& buffer)
{
    return read_helper(path, buffer);
}

std::wstring VT::folder_name( const std::wstring& filename, bool trimTrailingSlash /*= false*/ )
{
    auto pos = filename.find_last_of(VT_SLASH);
    if (pos != filename.npos)
        return filename.substr(0, pos + (trimTrailingSlash ? 0 : 1));
    else
        return filename;
}

std::wstring VT::file_name(const std::wstring& full_path)
{
    auto pos = full_path.find_last_of(VT_SLASH);
    if (pos != full_path.npos)
        return full_path.substr(pos + 1);
    else
        return full_path;
}
