#pragma once

#include <memory>
#include <functional>
#include <ostream>
#include <cassert>

namespace VT
{

    template<class T>
    class PList
    {
    public:
        // Empty list
        PList() {}

        // Cons
        PList(T v, const PList& tail)
            : _head(std::make_shared<Item>(v, tail._head)) {}

        bool isEmpty() const { return !_head; }

        T front() const
        {
            assert(!isEmpty());
            return _head->_val;
        }

        PList pop_front() const
        {
            assert(!isEmpty());
            return PList(_head->_next);
        }

        // Additional utilities
        PList push_front(T v) const
        {
            return PList(v, *this);
        }

        PList insertAt(int i, T v) const
        {
            if (i == 0)
            {
                return push_front(v);
            }
            else
            {
                assert(!isEmpty());
                return PList(front(), pop_front().insertAt(i - 1, v));
            }
        }

    private:
        struct Item
        {
            Item(T v, const std::shared_ptr<const Item>& tail)
                : _val(v), _next(tail) {}

            T _val;
            std::shared_ptr<const Item> _next;
        };
        friend struct Item;

        explicit PList (const Item* items) : _head(items) {}

        // Old C++ trick to encode a Maybe value
        std::shared_ptr<const Item> _head;
    };

    template<class T>
    PList<T> concat(PList<T> a, PList<T> b)
    {
        if (a.isEmpty())
            return b;

        return PList<T>(a.front(), concat(a.pop_front(), b));
    }

    template<class U, class T, class F>
    PList<U> fmap(F f, PList<T> lst)
    {
        static_assert(std::is_convertible<F, std::function<U(T)>>::value,
                     "fmap requires a function type U(T)");

        if (lst.isEmpty())
            return PList<U>();
        else
            return PList<U>(f(lst.front()), fmap<U>(f, lst.pop_front()));
    }

    template<class T, class P>
    PList<T> filter(P p, PList<T> lst)
    {
        static_assert(std::is_convertible<P, std::function<bool(T)>>::value,
                     "filter requires a function type bool(T)");

        if (lst.isEmpty())
            return PList<T>();

        if (p(lst.front()))
            return PList<T>(lst.front(), filter(p, lst.pop_front()));
        else
            return filter(p, lst.pop_front());
    }

    template<class T, class U, class F>
    U foldr(F f, U acc, PList<T> lst)
    {
        static_assert(std::is_convertible<F, std::function<U(T, U)>>::value,
                     "foldr requires a function type U(T, U)");

        if (lst.isEmpty())
            return acc;
        else
            return f(lst.front(), foldr(f, acc, lst.pop_front()));
    }

    template<class T, class U, class F>
    U foldl(F f, U acc, PList<T> lst)
    {
        static_assert(std::is_convertible<F, std::function<U(U, T)>>::value,
                     "foldl requires a function type U(U, T)");

        if (lst.isEmpty())
            return acc;
        else
            return foldl(f, f(acc, lst.front()), lst.pop_front());
    }

    template<class T, class F>
    void forEach(PList<T> lst, F f)
    {
        static_assert(std::is_convertible<F, std::function<void(T)>>::value,
                     "forEach requires a function type void(T)");

        if (!lst.isEmpty())
        {
            f(lst.front());
            forEach(lst.pop_front(), f);
        }
    }

    template<class Beg, class End>
    auto fromIt(Beg it, End end) -> PList<typename Beg::value_type>
    {
        typedef typename Beg::value_type T;

        if (it == end)
            return PList<T>();

        T item = *it;
        return PList<T>(item, fromIt(++it, end));
    }

    template<class T>
    std::ostream& operator<<(std::ostream& strm, PList<T> lst)
    {
        forEach(lst, [&strm](T v)
        {
            strm << "(" << v << ") ";
        });
    }

}
