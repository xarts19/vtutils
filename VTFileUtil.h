#pragma once

#include <string>

namespace VT
{
    class Path
    {
    };

    // suitable only for reading small text files into a buffer
    bool read_file(const std::string& path, std::string& buffer);

    std::string full_path(const std::string& filename);
}


