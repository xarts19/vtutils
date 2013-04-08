#include "VTNet.h"

#include "VTUtil.h"
#include "VTStringUtil.h"

#include <stdexcept>
#include <iostream>
#include <memory>

#include <Ws2tcpip.h>
#include <winsock.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")

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


std::string VT::Net::subnet_mask(const sf::IpAddress& ip)
{
    std::string mask;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    IP_ADAPTER_INFO* pAdapterInfo =(IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    std::shared_ptr<IP_ADAPTER_INFO> adapter_info;
    if (pAdapterInfo == NULL)
    {
        return mask;
    }

    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        adapter_info = std::shared_ptr<IP_ADAPTER_INFO>((IP_ADAPTER_INFO*)malloc(ulOutBufLen), free);
        if (pAdapterInfo == NULL)
        {
            return mask;
        }
    }
    else
    {
        adapter_info = std::shared_ptr<IP_ADAPTER_INFO>(pAdapterInfo, free);
    }

    if (GetAdaptersInfo(adapter_info.get(), &ulOutBufLen) == NO_ERROR)
    {
        IP_ADDR_STRING* ip_struct = &adapter_info->IpAddressList;
        while(ip_struct)
        {
            if (ip_struct->IpAddress.String == ip.toString())
            {
                mask = ip_struct->IpMask.String;
                break;
            }

            ip_struct = ip_struct->Next;
        }
    }

    return mask;
}


int VT::Net::last_error()
{
    return WSAGetLastError();
}


VT::SubnetIterator::SubnetIterator(sf::IpAddress ip)
{
    auto mask_str = VT::Net::instance().subnet_mask(ip);
    unsigned int my_ip_int = ip.toInteger();
    unsigned int mask = sf::IpAddress(mask_str).toInteger();
    not_mask_ = ~mask;
    net_base_addr_ = my_ip_int & mask;
    ip_counter_ = 0;
}


sf::IpAddress VT::SubnetIterator::next()
{
    if (!done())
    {
        return sf::IpAddress(net_base_addr_ | ip_counter_++);
    }
    else
    {
        return sf::IpAddress::None;
    }
}


int VT::SubnetIterator::size() const
{
    return not_mask_ + 1;
}


bool VT::SubnetIterator::done() const
{
    return ip_counter_ > not_mask_;
}