#pragma once

#include <vector>
#include <string>

namespace VT
{
    class Net
    {
    public:
        ~Net();

        static Net& instance()
        {
            static Net n;
            return n;
        }

        std::string hostname();
        std::vector<std::string> ip_list();
        bool is_it_my_ip(const std::string& ip);
        bool ip_valid(const std::string& ip);

    private:
        Net();
    };
}
