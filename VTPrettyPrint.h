#pragma once

#include <stddef.h>
#include <iostream>
#include <iterator>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <memory>

#if defined(_MSC_VER)
    #define TUPLE_PARAMS \
        typename T0, typename T1, typename T2, typename T3, typename T4
    #define TUPLE_ARGS T0, T1, T2, T3, T4
#else
    #define TUPLE_PARAMS typename... Types
    #define TUPLE_ARGS Types...
#endif

struct default_formatter
{
    template <typename T>
    void prefix(std::ostream& os, const T&) const { os << "["; }
    template <typename T>
    void separator(std::ostream& os, const T&) const { os << ", "; }
    template <typename T>
    void suffix(std::ostream& os, const T&) const { os << "]"; }

    template <typename A, typename B>
    void prefix(std::ostream& os, const std::pair<A, B>&) const { os << "("; }
    template <typename A, typename B>
    void separator(std::ostream& os, const std::pair<A, B>&) const { os << ", "; }
    template <typename A, typename B>
    void suffix(std::ostream& os, const std::pair<A, B>&) const { os << ")"; }

    template <TUPLE_PARAMS>
    void prefix(std::ostream& os, const std::tuple<TUPLE_ARGS>&) const { os << "("; }
    template <TUPLE_PARAMS>
    void separator(std::ostream& os, const std::tuple<TUPLE_ARGS>&) const { os << ", "; }
    template <TUPLE_PARAMS>
    void suffix(std::ostream& os, const std::tuple<TUPLE_ARGS>&) const { os << ")"; }

    template <typename K, typename C, typename A>
    void prefix(std::ostream& os, const std::set<K, C, A>&) const { os << "{"; }
    template <typename K, typename C, typename A>
    void separator(std::ostream& os, const std::set<K, C, A>&) const { os << ", "; }
    template <typename K, typename C, typename A>
    void suffix(std::ostream& os, const std::set<K, C, A>&) const { os << "}"; }

    template <typename T>
    void prefix(std::ostream& os, const std::shared_ptr<T>&) const { os << "shared_ptr("; }
    template <typename T>
    void suffix(std::ostream& os, const std::shared_ptr<T>&) const { os << ")"; }

    template <typename T, typename D>
    void prefix(std::ostream& os, const std::unique_ptr<T, D>&) const { os << "unique_ptr("; }
    template <typename T, typename D>
    void suffix(std::ostream& os, const std::unique_ptr<T, D>&) const { os << ")"; }

    template <typename T>
    void element(std::ostream& os, const T& t) const
    {
        os << t;
    }

    template <typename Ch, typename Tr, typename Al>
    void element(std::ostream& os, const std::basic_string<Ch, Tr, Al>& s) const
    {
        os << "\"" << s << "\"";
    }
};

namespace detail_
{
    
    using std::begin;
    using std::end;

    template <typename T>
    struct is_container_helper
    {
        template <typename U>
        static  std::true_type f(typename U::const_iterator *);
        template <typename U>
        static std::false_type f(...);

        typedef decltype(f<T>(0)) type;
    };

    template <typename T>
    struct is_container
        : public is_container_helper<T>::type { };

    template <typename T, size_t N>
    struct is_container<T[N]>
        : public std::true_type { };

    template <size_t N>
    struct is_container<const char[N]>
        : public std::false_type { };

    template <size_t N>
    struct is_container<char[N]>
        : public std::false_type { };

    template <typename Ch, typename Tr, typename Al>
    struct is_container<std::basic_string<Ch, Tr, Al>>
        : public std::false_type { };


    template <typename T, typename Fmt>
    void print(std::ostream& os, const T& t, const Fmt& fmt);

    template <typename A, typename B, typename Fmt>
    void print(std::ostream& os, const std::pair<A, B>& p, const Fmt& fmt);

    template <TUPLE_PARAMS, typename Fmt>
    void print(std::ostream& os, const std::tuple<TUPLE_ARGS>& t, const Fmt& fmt);

    template <typename T, typename Fmt>
    void print(std::ostream& os, const std::unique_ptr<T>& t, const Fmt& fmt);

    template <typename T, typename Fmt>
    void print(std::ostream& os, const std::shared_ptr<T>& t, const Fmt& fmt);


    template <typename Tuple, typename Fmt, size_t I>
    void print_tuple_helper(std::ostream& os, const Tuple& t, const Fmt& fmt,
                            std::integral_constant<size_t, I>);

    template <typename Tuple, typename Fmt>
    void print_tuple_helper(std::ostream& os, const Tuple& t, const Fmt& fmt,
                            std::integral_constant<size_t, 1>);

    template <typename Tuple, typename Fmt>
    void print_tuple_helper(std::ostream& os, const Tuple& t, const Fmt& fmt,
                            std::integral_constant<size_t, 0>);


    template <typename C, typename Fmt>
    void print_container_helper(std::ostream& os, const C& c, std::true_type, const Fmt& fmt);

    template <typename T, typename Fmt>
    void print_container_helper(std::ostream& os, const T& t, std::false_type, const Fmt& fmt);


    template <typename T, typename Fmt>
    void print(std::ostream& os, const T& t, const Fmt& fmt)
    {
        print_container_helper(os, t, typename is_container<T>::type(), fmt);
    }

    template <typename A, typename B, typename Fmt>
    void print(std::ostream& os, const std::pair<A, B>& p, const Fmt& fmt)
    {
        fmt.prefix(os, p);
        print(os, p.first, fmt);
        fmt.separator(os, p);
        print(os, p.second, fmt);
        fmt.suffix(os, p);
    }

    template <TUPLE_PARAMS, typename Fmt>
    void print(std::ostream& os, const std::tuple<TUPLE_ARGS>& t, const Fmt& fmt)
    {
        const size_t N = std::tuple_size<std::tuple<TUPLE_ARGS>>::value;
        fmt.prefix(os, t);
        print_tuple_helper(os, t, fmt, std::integral_constant<size_t, N>());
        fmt.suffix(os, t);
    }

    template <typename T, typename Fmt>
    void print(std::ostream& os, const std::unique_ptr<T>& t, const Fmt& fmt)
    {
        fmt.prefix(os, t);
        if (t)
            print(os, *t, fmt);
        fmt.suffix(os, t);
    }

    template <typename T, typename Fmt>
    void print(std::ostream& os, const std::shared_ptr<T>& t, const Fmt& fmt)
    {
        fmt.prefix(os, t);
        if (t)
            print(os, *t, fmt);
        fmt.suffix(os, t);
    }


    template <typename Tuple, typename Fmt, size_t I>
    void print_tuple_helper(std::ostream& os, const Tuple& t, const Fmt& fmt,
                            std::integral_constant<size_t, I>)
    {
        const size_t N = std::tuple_size<Tuple>::value;
        print(os, std::get<N - I>(t), fmt);
        fmt.separator(os, t);
        print_tuple_helper(os, t, fmt, std::integral_constant<size_t, I - 1>());
    }

    template <typename Tuple, typename Fmt>
    void print_tuple_helper(std::ostream& os, const Tuple& t, const Fmt& fmt,
                            std::integral_constant<size_t, 1>)
    {
        const size_t N = std::tuple_size<Tuple>::value;
        print(os, std::get<N - 1>(t), fmt);
    }

    template <typename Tuple, typename Fmt>
    void print_tuple_helper(std::ostream&, const Tuple&, const Fmt&,
                            std::integral_constant<size_t, 0>) { }


    template <typename C, typename Fmt>
    void print_container_helper(std::ostream& os, const C& c, std::true_type, const Fmt& fmt)
    {
        fmt.prefix(os, c);

        auto i = begin(c);
        auto e = end(c);

        if (i != e) {
            for (;;) {
                print(os, *i, fmt);

                if (++i != e) {
                    fmt.separator(os, c);
                } else {
                    break;
                }
            }
        }

        fmt.suffix(os, c);
    }

    template <typename T, typename Fmt>
    void print_container_helper(std::ostream& os, const T& t, std::false_type, const Fmt& fmt)
    {
        fmt.element(os, t);
    }

}

template <typename T, typename = typename std::enable_if<detail_::is_container<T>::value>::type>
std::ostream& operator<<(std::ostream& os, const T& t)
{
    detail_::print_container_helper(os, t, typename detail_::is_container<T>::type(), default_formatter());
    return os;
}

template <typename A, typename B>
std::ostream& operator<<(std::ostream& os, const std::pair<A, B>& p)
{
    detail_::print(os, p, default_formatter());
    return os;
}

template <TUPLE_PARAMS>
std::ostream& operator<<(std::ostream& os, const std::tuple<TUPLE_ARGS>& t)
{
    detail_::print(os, t, default_formatter());
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::unique_ptr<T>& t)
{
    detail_::print(os, t, default_formatter());
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& t)
{
    detail_::print(os, t, default_formatter());
    return os;
}

template <typename T, typename Fmt>
void print_line(std::ostream& os, const T& t, const Fmt& fmt)
{
    detail_::print(os, t, fmt);
    os << std::endl;
}

