#include "VTNet.h"

#include "VTUtil.h"
#include "VTStringUtil.h"

#include <stdexcept>
#include <iostream>

#include <Ws2tcpip.h>
#include <winsock.h>

VT::Net::Net()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        throw std::runtime_error("WSAStartup failed");
    }
}


VT::Net::~Net()
{
    WSACleanup();
}


std::string VT::Net::hostname()
{
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
    {
        throw std::runtime_error(VT::strerror(WSAGetLastError()));
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


bool VT::Net::ip_valid(const std::string& ip)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
    return result == 1;
}
