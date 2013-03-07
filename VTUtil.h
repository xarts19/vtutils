#pragma once

#include <vector>
#include <algorithm>

#define VT_DISABLE_COPY(Class) Class(const Class &); Class & operator= (const Class &);
#define VT_UNUSED( x ) ( &reinterpret_cast< const int& >( x ) )

namespace VT
{
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
    };
};


