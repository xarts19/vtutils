#pragma once

#include "SFML/Network/IpAddress.hpp"

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
        std::string subnet_mask(const sf::IpAddress& ip);

        int last_error();

    private:
        Net();
    };


    class SubnetIterator
    {
    public:
        SubnetIterator(sf::IpAddress ip);

        // returns sf::IpAddress::None when done
        sf::IpAddress next();
        int size() const;
        bool done() const;

    private:
        unsigned int ip_counter_;
        unsigned int net_base_addr_;
        unsigned int not_mask_;
    };
}
