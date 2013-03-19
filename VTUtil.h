#pragma once

#include <vector>
#include <algorithm>
#include <string>

#define VT_DISABLE_COPY(Class) Class(const Class &); Class & operator= (const Class &);
#define VT_UNUSED( x ) do { (void)sizeof(x); } while(0)

namespace VT
{
    std::string strerror(int err_code);
    
    namespace Utils
    {
        template <typename T>
        bool insert_if_not_present(std::vector<T>& container, T value)
        {
            if ( std::find(container.begin(), container.end(), value ) == container.end() )
            {
                container.push_back(value);
                return true;
            }
            return false;
        }
    }
}


