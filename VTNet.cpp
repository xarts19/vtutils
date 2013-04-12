#include "VTNet.h"

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