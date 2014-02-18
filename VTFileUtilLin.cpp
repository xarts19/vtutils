#include "VTFileUtil.h"

#include "VTUtil.h"
#include "VTStringUtil.h"
#include "VTEncodeConvert.hpp"

#include <stdexcept>
#include <cassert>

#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stddef.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <fcntl.h>


std::string full_path_utf8(const std::string& filename)
{
    char buffer[PATH_MAX];
    char* ptr = realpath(filename.c_str(), buffer);

    if (ptr == 0)
        throw std::runtime_error(filename + ": " + VT::strerror(errno));
    else
        return buffer;
}

std::wstring VT::full_path(const std::wstring& filename)
{
    return VT::UTF8ToWstring(full_path_utf8(VT::WstringToUTF8(filename)));
}

bool file_exists_utf8(const std::string& filename)
{
    struct stat sb;
    return (0 == stat(filename.c_str(), &sb));
}

bool VT::file_exists(const std::wstring& filename)
{
    return file_exists_utf8(VT::WstringToUTF8(filename));
}

bool VT::ensure_path_exist(const std::wstring& path)
{
    std::string folder_utf8 = VT::WstringToUTF8(path.substr(0, path.rfind(L'/')));

    size_t index = 0;
    size_t pos = 0;

    while ((pos = folder_utf8.find('/', index)) != folder_utf8.npos)
    {
        auto dir_name = folder_utf8.substr(0, pos);

        if (!file_exists_utf8(dir_name))
        {
            if (mkdir(dir_name.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST)
                return false;
        }

        index = pos + 1;
    }

    return true;
}

bool is_directory(const std::string& path)
{
    struct stat s_buf;

    if (stat(path.c_str(), &s_buf) != 0)
    {
        throw std::runtime_error("Failed reading '" + path + "': " + VT::strerror(VT::last_error()));
    }

    return S_ISDIR(s_buf.st_mode);
}

void delete_file(const std::string& path)
{
    if (unlink(path.c_str()) != 0)
        throw std::runtime_error("Failed deleting file '" + path + "': " + VT::strerror(VT::last_error()));
}

void delete_folder(const std::string& folder)
{
    std::unique_ptr<DIR, std::function<decltype(closedir)>> dp(opendir(folder.c_str()), closedir);
    if (!dp)
        throw std::runtime_error("Failed opening folder '" + folder + "': " + VT::strerror(VT::last_error()));

    struct dirent* ep;

    while ((ep = readdir(dp.get())) != NULL)
    {
        std::string path = folder + "/" + ep->d_name;

        if (is_directory(path))
            delete_folder(path);
        else
            delete_file(path);
    }

    if (rmdir(folder.c_str()) != 0)
        throw std::runtime_error("Failed deleting folder '" + folder + "': " + VT::strerror(VT::last_error()));
}

bool VT::delete_file_folder(std::wstring path)
{
    try
    {
        std::vector<std::wstring> files_to_delete;
        VT::tsplit(path, &files_to_delete, std::wstring(L"\0"));

        for (const auto& wpath : files_to_delete)
        {
            std::string utf8 = VT::WstringToUTF8(wpath);
            if (is_directory(utf8))
                delete_folder(utf8);
            else
                delete_file(utf8);
        }
    }
    catch (const std::exception&)
    {
        return false;
    }

    return true;
}

namespace FileType
{
    enum FileType
    {
        File = 0x01,
        Dir = 0x02,
        Unknown = 0x04,
        Both = File | Dir
    };
}

void TraverseDirectory(const std::wstring& directory,
                       const std::wstring& mask,
                       FileType::FileType file_type,
                       std::list<std::wstring>& result)
{
    errno = 0;
    auto directory_utf8 = VT::WstringToUTF8(directory);
    if (directory_utf8.back() != VT_SLASH)
        directory_utf8.push_back(VT_SLASH);

    auto mask_utf8 = VT::WstringToUTF8(mask);

    DIR* dir = opendir(directory_utf8.c_str());
    if (!dir)
        return;

    /*
    long name_max = pathconf(directory_utf8.c_str(), _PC_NAME_MAX);
    if (name_max == -1)         // Limit not defined, or error
        name_max = 255;         // Take a guess
    size_t len = offsetof(struct dirent, d_name) + name_max + 1;
    struct dirent* entry = malloc(len);
    */
    struct dirent64 entry;
    struct dirent64* entryp = nullptr;

    int ret = 0;
    while ((ret = readdir64_r(dir, &entry, &entryp)) == 0)
    {
        if (!entryp)
            break;  // end of directory

        if (0 == fnmatch(mask_utf8.c_str(), entryp->d_name, FNM_PERIOD))
        {
            // name matches the mask
            auto rel_name = directory_utf8 + entryp->d_name;
            struct stat s;
            if (0 == stat(rel_name.c_str(), &s))
            {
                FileType::FileType ft;
                if (S_ISREG(s.st_mode))
                {
                    ft = FileType::File;
                }
                else if (S_ISDIR(s.st_mode))
                {
                    ft = FileType::Dir;
                }
                else
                {
                    ft = FileType::Unknown;
                }

                if (file_type & ft)
                {
                    // matches needed type
                    result.push_back(VT::UTF8ToWstring(full_path_utf8(rel_name)));
                }
            }
            else
            {
                //int why = errno;
                //auto what = VT::strerror(why);
                //int unused = 0;
                // just skip broken or restricted files
            }
        }
    }

    if (ret != 0)
        throw std::runtime_error(directory_utf8 + ": " + VT::strerror(ret));

    ret = closedir(dir);
    assert(ret == 0);
    VT_UNUSED(ret);
}

size_t VT::list_files( const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result )
{
    TraverseDirectory(directory, mask, FileType::File, result);
    return result.size();
}

size_t VT::list_folders( const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result )
{
    TraverseDirectory(directory, mask, FileType::Dir, result);
    return result.size();
}

size_t VT::list_files_folders( const std::wstring& directory, const std::wstring& mask, std::list<std::wstring>& result )
{
    TraverseDirectory(directory, mask, FileType::Both, result);
    return result.size();
}



struct VT::File::Impl
{
    Impl()
        : hFile(-1)
    { }

    int hFile;
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
    int fl = O_RDONLY;

    if(!(flags & VT::File::ReadOnly))
    {
        fl = O_RDWR;
    }

    // Prevent handle leak
    Close();
    _pImpl->hFile = open(VT::WstringToUTF8(filename).c_str(), fl);
    return (_pImpl->hFile != -1);
}

bool VT::File::Create(const std::wstring& filename, VT::File::Flags /*flags*/)
{
    // Prevent handle leak
    Close();
    ensure_path_exist(filename);
    _pImpl->hFile = creat(VT::WstringToUTF8(filename).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    return (_pImpl->hFile != -1);
}

void VT::File::Close()
{
    if(_pImpl->hFile != -1)
    {
        close(_pImpl->hFile);
        _pImpl->hFile = -1;
    }
}

bool VT::File::Read( void* data, uint32_t size, int64_t pos /*= -1*/ )
{
    if(_pImpl->hFile == -1)
        return false;

    // Set pos
    if (pos != -1)
    {
        if (lseek64(_pImpl->hFile, pos, SEEK_SET) ==  (off64_t)-1)
            return false;
    }

    if (read(_pImpl->hFile, data, size) == size)
        return true;
    else
        return false;
}

bool VT::File::Write( const void* data, uint32_t size, int64_t pos /*= -1*/ )
{
    if(_pImpl->hFile == -1)
        return false;

    // Set pos
    if (pos != -1)
    {
        if (lseek64(_pImpl->hFile, pos, SEEK_SET) ==  (off64_t)-1)
            return false;
    }

    if (write(_pImpl->hFile, data, size) == size)
        return true;
    else
        return false;

}

int64_t VT::File::ReadAll(std::unique_ptr<uint8_t[]>& data)
{
    uint32_t size = (uint32_t)this->size();
    data.reset(new uint8_t[size]);

    if(this->Read(data.get(), size, 0))
        return size;
    else
        return -1;
}

int64_t VT::File::size()
{
    if(_pImpl->hFile == -1)
        return -1;

    struct stat sb;

    if (fstat(_pImpl->hFile, &sb) != 0)
        return -1;

    return sb.st_size;
}
