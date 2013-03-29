#include "VTFileUtil.h"

#include <fstream>

bool VT::read_file(const std::string& path, std::string& buffer)
{
    std::ifstream f(path);

    if (!f.good())
        return false;

    buffer.assign((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());
    return true;
}
