
namespace VT
{
    namespace Utils
    {
        namespace File
        {
            bool read_file(const char* path, std::string& buffer)
            {
                std::ifstream t(path);
                if ( !t.good() )
                    return false;

                t.seekg(0, std::ios::end);
                size_t size = t.tellg();
                if (size == -1)
                    return false;

                buffer.resize(size);
                t.seekg(0);
                t.read(&buffer[0], size); 
                if ( !t.good() )
                    return false;

                return true;
            }
        };
    };
};


