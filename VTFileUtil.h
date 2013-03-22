#pragma once

#include <string>

namespace VT
{
    namespace FileUtil
    {
        bool read_file(const std::string& path, std::string& buffer);

        std::string full_path(const std::string& filename);
    }
}


