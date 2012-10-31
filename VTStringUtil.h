
namespace VT
{
    namespace Utils
    {
        namespace String
        {
            template <typename Container>
            void split(Container&                            result,
                       const typename Container::value_type& s,
                       const typename Container::value_type& delimiters)
            {
                result.clear();
                size_t current;
                size_t next = -1;
                do
                {
                    next = s.find_first_not_of( delimiters, next + 1 );
                    if (next == Container::value_type::npos) break;
                    next -= 1;

                    current = next + 1;
                    next = s.find_first_of( delimiters, current );
                    result.push_back( s.substr( current, next - current ) );
                }
                while (next != Container::value_type::npos);
            }

            std::string trim(const std::string& str,
                             const std::string& whitespace)
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

            char _char_to_lower(char in)
            {
                return static_cast<char>( ::tolower(in) );
            } 

            std::string to_lower(std::string str)
            {
                std::transform(str.begin(), str.end(), str.begin(), _char_to_lower);
                return str;
            }
        };
    };
};


