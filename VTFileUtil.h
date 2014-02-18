#pragma once

#include "VTCompilerSpecific.h"

#include <stdint.h>
#include <string>
#include <list>
#include <vector>
#include <memory>

#ifdef COMPILER_MSVC
    #define VT_SLASH L'\\'
    #define VT_SLASH_STR L"\\"
#else
    #define VT_SLASH L'/'
    #define VT_SLASH_STR L"/"
#endif

#ifdef COMPILER_MSVC
    #define VT_SLASH_UTF8 '\\'
    #define VT_SLASH_STR_UTF8 "\\"
#else
    #define VT_SLASH_UTF8 '/'
    #define VT_SLASH_STR_UTF8 "/"
#endif

namespace VT
{
    class Path
    {
    };

    // suitable only for reading small text files into a buffer
    bool read_file(const std::wstring& path, std::string& buffer);
    bool read_file(const std::wstring& path, std::vector<char>& buffer);
    bool read_file(const std::wstring& path, std::wstring& buffer);
    bool read_file(const std::wstring& path, std::vector<wchar_t>& buffer);

    std::wstring file_name(const std::wstring& full_path);
    std::wstring folder_name(const std::wstring& filename, bool trimTrailingSlash = false);

    std::wstring full_path(const std::wstring& filename);
    bool file_exists(const std::wstring& filename);
    bool ensure_path_exist(const std::wstring& path);
    bool delete_file_folder(std::wstring path);
    bool move_file(const std::wstring& filename, const std::wstring& newFile);

    size_t list_files(const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result);
    size_t list_folders(const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result);
    size_t list_files_folders(const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result);

    // Generic file data operations
    class File
    {
    public:
        enum Flags
        {
            None = 0,
            ReadOnly,       // File will be opened in read only mode
            Sequential,     // Optimize for sequential access if possible
        };

    public:
        File();
        ~File();

        bool Open(const std::wstring& filename, Flags flags = None);
        bool Create(const std::wstring& filename, Flags flags = None);
        void Close();

        bool Read(void* data, uint32_t size, int64_t pos = -1);
        bool Write(const void* data, uint32_t size, int64_t pos = -1);

        int64_t ReadAll(std::unique_ptr<uint8_t[]>& data);

        int64_t size();

    private:
        File(const File&);
        File& operator=(const File&);

        struct Impl;
        Impl* _pImpl;
    };
}


