
namespace VTUtils
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


