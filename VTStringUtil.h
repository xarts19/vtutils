#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

namespace VT
{
    namespace StrUtils
    {
        // delimiters is a string of delimiting characters of length 1
        template <typename Container>
        void split(const std::string& str,
                   Container*         result,
                   const std::string& delimiters = " ")
        {
            size_t current;
            size_t next = static_cast<size_t>(-1);
            do
            {
                next = str.find_first_not_of( delimiters, next + 1 );
                if (next == std::string::npos) break;
                next -= 1;

                current = next + 1;
                next = str.find_first_of( delimiters, current );
                result->push_back( str.substr( current, next - current ) );
            }
            while (next != std::string::npos);
        }

        std::string trim(const std::string& str,
                         const std::string& whitespace = " ")
        {
            const size_t strBegin = str.find_first_not_of(whitespace);
            if (strBegin == std::string::npos)
                return ""; // no content

            const size_t strEnd = str.find_last_not_of(whitespace);
            const size_t strRange = strEnd - strBegin + 1;

            return str.substr(strBegin, strRange);
        }

        bool starts_with(const std::string& str,
                         const std::string& prefix)
        {
            if ( str.substr( 0, prefix.size() ) == prefix )
                return true;
            return false;
        }

        namespace detail_
        {
            char char_to_lower(char in)
            {
                return static_cast<char>( std::tolower(in) );
            }
        }

        std::string to_lower(std::string str)
        {
            std::transform(str.begin(), str.end(), str.begin(), detail_::char_to_lower);
            return str;
        }
    }
}


