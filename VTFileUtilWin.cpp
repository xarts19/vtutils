#include "VTFileUtil.h"

#include "VTUtil.h"
#include "VTEncodeConvert.hpp"

#include <windows.h>
#include <ShlObj.h>


std::wstring VT::full_path(const std::wstring& filename)
{
    WCHAR buffer[4096];

    DWORD retval = GetFullPathNameW(filename.c_str(), 4096, buffer, nullptr);

    if (retval == 0)
    {
        throw std::runtime_error(VT::WstringToUTF8(filename) + ": " + VT::strerror(GetLastError()));
    }
    else
    {
        return buffer;
    }
}


bool VT::ensure_path_exist( const std::wstring& path )
{
    std::wstring folder = path.substr(0, path.rfind(VT_SLASH));
    SHCreateDirectoryExW(NULL, folder.c_str(), NULL);

    return true;
}

bool VT::file_exists( const std::wstring& filename )
{
    return (GetFileAttributesW(filename.c_str()) != INVALID_FILE_ATTRIBUTES);
}

bool VT::delete_file_folder(std::wstring path)
{
    SHFILEOPSTRUCTW fileOp = {0};

    // Double null-terminated string
    path.push_back(L'\0');
    path.push_back(L'\0');

    fileOp.fFlags = FOF_NO_UI;
    fileOp.pFrom  = path.c_str();
    fileOp.wFunc  = FO_DELETE;
    fileOp.hwnd   = NULL;

    return (SHFileOperationW(&fileOp) == 0);
}

bool VT::move_file(const std::wstring& filename, const std::wstring& newFile)
{
    return (MoveFileExW(filename.c_str(), newFile.c_str(), MOVEFILE_REPLACE_EXISTING) == TRUE);
}

size_t TraverseDirectory( const std::wstring& directory, 
                          const std::wstring& mask, 
                          uint32_t exludeFlags, 
                          uint32_t includeFlags, 
                          std::list<std::wstring>& result )
{
    std::wstring searchPath = directory + VT_SLASH + mask;
    WIN32_FIND_DATAW ffd = {0};
    HANDLE hFind = NULL;

    hFind = FindFirstFileW(searchPath.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        return 0;
    }
    else 
    {
        do
        {
            // Skip . and .. directories
            if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::wstring name(ffd.cFileName);
                if(name == L"." || name == L"..")
                    continue;
            }

            // Filter results
            if((ffd.dwFileAttributes & includeFlags) || (!(ffd.dwFileAttributes & exludeFlags)))
                result.emplace_back(directory + VT_SLASH + ffd.cFileName);
        }
        while (FindNextFileW(hFind, &ffd) != 0);

        FindClose(hFind);

        return result.size();
    }
}

size_t VT::list_files( const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result )
{
    return TraverseDirectory(directory, mask, FILE_ATTRIBUTE_DIRECTORY, 0xFFFFFFFF, result);
}

size_t VT::list_folders( const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result )
{
    return TraverseDirectory(directory, mask, 0, FILE_ATTRIBUTE_DIRECTORY, result);
}

size_t VT::list_files_folders( const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result )
{
    return TraverseDirectory(directory, mask, 0, 0xFFFFFFFF, result);
}


struct VT::File::Impl
{
    Impl()
        : hFile(INVALID_HANDLE_VALUE)
    { }

    HANDLE hFile;
};

VT::File::File()
    : _pImpl(new VT::File::Impl())
{
}

VT::File::~File()
{
    Close();
    delete _pImpl;
}

bool VT::File::Open(const std::wstring& filename, VT::File::Flags flags)
{
    DWORD access  = FILE_READ_ATTRIBUTES | FILE_READ_DATA;
    DWORD share   = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    DWORD attrbis = 0;

    // Parse flags
    if(flags & VT::File::Sequential)
        attrbis |= FILE_FLAG_SEQUENTIAL_SCAN;

    if(!(flags & VT::File::ReadOnly))
    {
        access |= FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA;
    }

    // Prevent handle leak
    Close();

    _pImpl->hFile = CreateFileW(filename.c_str(), access, share, NULL, OPEN_EXISTING, attrbis, NULL);

    return (_pImpl->hFile != INVALID_HANDLE_VALUE);
}

bool VT::File::Create(const std::wstring& filename, VT::File::Flags flags)
{
    DWORD access  = FILE_READ_ATTRIBUTES | FILE_READ_DATA;
    DWORD share   = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    DWORD attrbis = 0;

    // Parse flags
    if(flags & VT::File::Sequential)
        attrbis |= FILE_FLAG_SEQUENTIAL_SCAN;

    if(!(flags & VT::File::ReadOnly))
    {
        access |= FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA;
    }

    // Prevent handle leak
    Close();

    ensure_path_exist(filename);

    _pImpl->hFile = CreateFileW(filename.c_str(), access, share, NULL, CREATE_ALWAYS, attrbis, NULL);

    return (_pImpl->hFile != INVALID_HANDLE_VALUE);
}

void VT::File::Close()
{
    if(_pImpl->hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_pImpl->hFile);
        _pImpl->hFile = INVALID_HANDLE_VALUE;
    }
}

bool VT::File::Read( void* data, uint32_t size, int64_t pos /*= -1*/ )
{
    DWORD bytes = 0;
    LARGE_INTEGER li = {0};

    if(_pImpl->hFile == INVALID_HANDLE_VALUE)
        return false;

    li.QuadPart = pos;

    // Set pos
    if(pos != -1)
    {
        if(SetFilePointer(_pImpl->hFile, li.LowPart, &li.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return false;
    }

    if(ReadFile(_pImpl->hFile, data, size, &bytes, NULL) == TRUE && bytes == size)
        return true;
    else
        return false;
}

bool VT::File::Write( const void* data, uint32_t size, int64_t pos /*= -1*/ )
{
    DWORD bytes = 0;
    LARGE_INTEGER li = {0};

    if(_pImpl->hFile == INVALID_HANDLE_VALUE)
        return false;

    li.QuadPart = pos;

    // Set pos
    if(pos != -1)
    {
        if(SetFilePointer(_pImpl->hFile, li.LowPart, &li.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return false;
    }

    if(WriteFile(_pImpl->hFile, data, size, &bytes, NULL) == TRUE && bytes == size)
        return true;
    else
        return false;

}

int64_t VT::File::ReadAll(std::unique_ptr<uint8_t[]>& data)
{
    uint32_t size = (uint32_t)this->size();
    data.reset(new uint8_t[size]());

    if(this->Read(data.get(), size, 0))
        return size;
    else
        return -1;
}

int64_t VT::File::size()
{
    LARGE_INTEGER li = {0};

    if(_pImpl->hFile == INVALID_HANDLE_VALUE)
        return -1;

    if(GetFileSizeEx(_pImpl->hFile, &li))
        return li.QuadPart;
    else
        return -1;
}
