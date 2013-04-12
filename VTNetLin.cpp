#include "VTNet.h"

#include "VTUtil.h"
#include "VTStringUtil.h"

#include <stdexcept>
#include <iostream>
#include <memory>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>


VT::Net::Net()
{
}


VT::Net::~Net()
{
}


std::string VT::Net::hostname()
{
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == -1)
    {
        throw std::runtime_error(VT::strerror(errno));
    }
    return ac;
}


std::vector<std::string> VT::Net::ip_list()
{
    struct hostent *phe = gethostbyname(hostname().c_str());
    if (phe == 0)
        throw std::runtime_error("Failed to get host by current hostname");

    std::vector<std::string> ips;

    for (int i = 0; phe->h_addr_list[i] != 0; ++i)
    {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        ips.push_back(inet_ntoa(addr));
    }

    return ips;
}


bool VT::Net::is_it_my_ip(const std::string& ip)
{
    if (VT::StrUtils::to_lower(ip) == VT::StrUtils::to_lower(hostname()))
    {
        return true;
    }
    else
    {
        auto ips = ip_list();
        if (std::find(ips.begin(), ips.end(), ip) != ips.end())
            return true;
    }

    return false;
}


std::string VT::Net::subnet_mask(const sf::IpAddress& ip)
{
    std::string mask;

    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1)
    {
        // error
        return mask;
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later */

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        int family = ifa->ifa_addr->sa_family;
        
        if (family == AF_INET)
        {
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                continue;
            }
            
            if (host == ip.toString())
            {
                if (ifa->ifa_netmask == NULL)
                {
                    // error
                    break;
                }
                
                char mask_cstr[NI_MAXHOST];
                s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in),
                                mask_cstr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                
                if (s != 0)
                {
                    // error
                    break;
                }
                
                mask = mask_cstr;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    
    return mask;
}


int VT::Net::last_error()
{
    return errno;
}

