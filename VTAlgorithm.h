#pragma once

namespace VT
{
    // erase elements from the container 'items' for which the 'predicate' returns true
    template< typename ContainerT, typename PredicateT >
    void erase_if( ContainerT& items, const PredicateT& predicate )
    {
        for(auto it = items.begin(); it != items.end(); /* inside the loop */)
        {
            if (predicate(*it))
                it = items.erase(it);
            else
                ++it;
        }
    }
}

