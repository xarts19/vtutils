#include "VTFileUtil.h"

#include <fstream>

bool VT::FileUtil::read_file(const std::string& path, std::string& buffer)
{
    std::ifstream t(path);
    if ( !t.good() )
        return false;

    t.seekg(0, std::ios::end);
    auto size = t.tellg();

    if (static_cast<int>(size) == -1)
        return false;

    buffer.resize(static_cast<std::string::size_type>(size));
    t.seekg(0, std::ios::beg);

    t.read(&buffer[0], size); 
    if ( !t.good() )
        return false;

    return true;
}
