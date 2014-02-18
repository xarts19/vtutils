#include "VTOSProcess.h"

#include "VTEncodeConvert.hpp"
#include "VTUtil.h"

#include <sstream>
#include <cassert>
#include <stdexcept>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>

#define GLOBALVAR_FAIL_READ "Failed reading GlobalVar value"
#define GLOBALVAR_FAIL_WRITE "Failed writing GlobalVar value"
#define GLOBALVAR_FAIL_NOT_LOCKED "GlobalVar not locked"

#define VAR_BUFFER_SIZE 8

struct VT::GlobalVar::Impl
{
    Impl()
        : tmpfd(0)
        , locked(false)
    {}

    int tmpfd;
    bool locked;
};

VT::GlobalVar::GlobalVar( const std::wstring& name )
    : _name(name)
    , _pImpl(new Impl)
{
    std::stringstream tempfile_name;
    tempfile_name << "/tmp/ArCycle_globalvar_pid_" << getpid() << "_" << VT::WstringToUTF8(name);

    // only this user will have permissions for semaphore, because we use it only from current process
    _pImpl->tmpfd = open(tempfile_name.str().c_str(),
                            O_RDWR      |   // open the file for both read and write access
                            O_CREAT     |   // create file if it does not already exist
                            O_CLOEXEC   |   // close on execute
                            O_DSYNC     ,   // block in write() until data is written to disk
                            S_IRUSR     |   // user permission: read
                            S_IWUSR     );  // user permission: write
    if (_pImpl->tmpfd == -1)
    {
        delete _pImpl;
        VT::create_system_exception("Failed creating GlobalVar");
    }
}

VT::GlobalVar::~GlobalVar()
{
    assert(_pImpl->tmpfd);

    if (_pImpl->locked)
        unlock();

    int ret = close(_pImpl->tmpfd);
    if (ret != 0)
    {
        assert(false && "Failed closing GlobalVar");
    }
    delete _pImpl;
}

namespace
{
    bool read_val(int fd, void* buf)
    {
        ssize_t size_read = pread(fd, buf, VAR_BUFFER_SIZE, 0);

        if (size_read == -1)
            VT::create_system_exception(GLOBALVAR_FAIL_READ);

        if (size_read > 0)    // size_read == 0 means EOF
            assert(size_read == VAR_BUFFER_SIZE);

        return size_read > 0;
    }

    void write_val(int fd, void* buf)
    {
        ssize_t size_written = pwrite(fd, buf, VAR_BUFFER_SIZE, 0);

        if (size_written == -1)
            VT::create_system_exception(GLOBALVAR_FAIL_WRITE);

        assert(size_written == VAR_BUFFER_SIZE);
    }

    int64_t buf_to_val(const unsigned char (&buf)[VAR_BUFFER_SIZE])
    {
        int64_t val = 0;
        for (int i = 0; i < VAR_BUFFER_SIZE; ++i)
            val |= buf[i] << (i * 8);
        return val;
    }

    void val_to_buf(int64_t val, unsigned char buf[VAR_BUFFER_SIZE])
    {
        for (int i = 0; i < VAR_BUFFER_SIZE; ++i)
            buf[i] = static_cast<unsigned char>(static_cast<uint64_t>(val) >> (i * 8));
    }
}

bool VT::GlobalVar::OperateMapping(int32_t& count, int32_t mod_val)
{
    assert(_pImpl->locked && GLOBALVAR_FAIL_NOT_LOCKED);
    if (!_pImpl->locked)
        throw std::runtime_error(GLOBALVAR_FAIL_NOT_LOCKED);

    unsigned char inbuf[VAR_BUFFER_SIZE];
    bool value_read = read_val(_pImpl->tmpfd, inbuf);

    if (value_read)
        count = buf_to_val(inbuf);

    if (mod_val == 0)
        return value_read;

    count += mod_val;
    unsigned char outbuf[VAR_BUFFER_SIZE];
    val_to_buf(count, outbuf);
    write_val(_pImpl->tmpfd, outbuf);
    return true;
}

bool VT::GlobalVar::is_set()
{
    int32_t count = 0;

    if (OperateMapping(count, 0) && count > 0)
        return true;
    else
        return false;
}

bool VT::GlobalVar::reference()
{
    int32_t count = 0;

    if (OperateMapping(count, 1) && count > 0)
        return true;
    else
        return false;
}

bool VT::GlobalVar::dereference()
{
    int32_t count = 0;

    if (OperateMapping(count, -1))
        return true;
    else
        return false;
}

void VT::GlobalVar::lock()
{
    int ret = 0;
    while (0 != (ret = flock(_pImpl->tmpfd, LOCK_EX)) && errno == EINTR)
    { }

    if (ret != 0)
    {
        assert(false && "GlobalVar lock error");
        VT::create_system_exception("GlobalVar lock failed");
    }
    else
    {
        _pImpl->locked = true;
    }
}

void VT::GlobalVar::unlock()
{
    assert(_pImpl->locked && GLOBALVAR_FAIL_NOT_LOCKED);
    if (!_pImpl->locked)
        throw std::runtime_error(GLOBALVAR_FAIL_NOT_LOCKED);

    int ret = flock(_pImpl->tmpfd, LOCK_UN);
    if (ret != 0)
    {
        assert(false && "GlobalVar unlock error");
    }
    else
    {
        _pImpl->locked = false;
    }
}

