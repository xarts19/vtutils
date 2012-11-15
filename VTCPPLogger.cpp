#include "VTCPPLogger.h"

#include <exception>
#include <iostream>

#include <time.h>
#include <assert.h>

// sink output into void
std::shared_ptr<VT::detail_::onullstream> VT::detail_::dev_null_ = 
    std::shared_ptr<VT::detail_::onullstream>(new VT::detail_::onullstream());

namespace VT
{
    namespace detail_
    {
        struct LoggerData
        {
            LoggerData() :
                type_       (LogType::Noop),
                stream_     (dev_null_),
                lock_       (),
                use_lock_   (false),
                log_level_  (LogLevel::Critical),
                default_opts_(LogOpt::Default)
            { }

            VT::detail_::LogTypes type_;
            std::shared_ptr<std::ostream> stream_;
            std::shared_ptr<VT::CriticalSection> lock_;
            bool use_lock_;
            VT::LogLevels log_level_;
            unsigned int default_opts_;
        };
    }
}

using VT::detail_::LoggerData;
using VT::detail_::LogWorker;
using VT::CriticalSection;
using VT::LogLevels;

std::string createTimestamp()
{
    time_t     now      = time( 0 );
    struct tm  timeinfo = *localtime( &now );
    char       buf[80];
    strftime( buf, sizeof( buf ), "%d/%m/%y %H:%M:%S", &timeinfo );
    return buf;
}


const char* getLogLevel(VT::LogLevels l)
{
    switch (l)
    {
    case VT::LogLevels::Debug:
        return "Debug   ";
    case VT::LogLevels::Info:
        return "Info    ";
    case VT::LogLevels::Warning:
        return "Warning ";
    case VT::LogLevels::Error:
        return "Error   ";
    case VT::LogLevels::Critical:
        return "Critical";
    default:
        assert(0);
        return "Unknown ";
    }
}


LogWorker::LogWorker(
            std::ostream& stream,
            CriticalSection* lock,
            LogLevels msg_level,
            unsigned int options) :
    stream_(stream),
    lock_(lock),
    valid_(true),
    msg_level_(msg_level),
    options_(options)
{
    if (lock_) lock_->enter();

    if (not_set(LogOpt::NoTimestamp))
        *this << createTimestamp();

    if (is_set(LogOpt::NoSpace))
        *this << " ";

    if (not_set(LogOpt::NoPrefix))
        *this << getLogLevel(msg_level_);

    if (is_set(LogOpt::NoSpace))
        *this << " ";
}

VT::detail_::LogWorker::LogWorker(LogWorker&& other) :
    stream_(other.stream_),
    lock_(other.lock_),
    valid_(true)
{
    // set so that other worker won't invoke usual destructor operations
    other.valid_ = false;
}

VT::detail_::LogWorker::~LogWorker()
{
    if (valid_)
    {
        if (not_set(LogOpt::NoEndl))
            *this << std::endl;
        else if (not_set(LogOpt::NoFlush))
            *this << std::flush;
        if (lock_) lock_->leave();
    }
}

bool VT::detail_::LogWorker::is_set(LogOpts opt) const
{
    return ( (options_ & opt) == opt );
}
bool VT::detail_::LogWorker::not_set(LogOpts opt) const
{
    return ( (options_ & opt) != opt );
}
void VT::detail_::LogWorker::set(LogOpts opt)
{
    options_ |= opt;
}

VT::detail_::LogWorker&
    VT::detail_::LogWorker::nospace()
{
    set(LogOpt::NoSpace);
    return *this;
}

VT::detail_::LogWorker&
    VT::detail_::LogWorker::noendl()
{
    set(LogOpt::NoEndl);
    return *this;
}

VT::detail_::LogWorker&
    VT::detail_::LogWorker::noprefix()
{
    set(LogOpt::NoPrefix);
    return *this;
}

VT::detail_::LogWorker&
    VT::detail_::LogWorker::notimestamp()
{
    set(LogOpt::NoTimestamp);
    return *this;
}

VT::detail_::LogWorker&
    VT::detail_::LogWorker::operator<<(std::ostream& (*manip)(std::ostream&))
{
    manip(stream_);
	return *this;
}

VT::detail_::LogWorker&
    VT::detail_::LogWorker::operator<<(std::ios_base& (*manip)(std::ios_base&))
{
    manip(stream_);
	return *this;
}


// usual logger
VT::Logger::Logger(
        const std::string& name,
        detail_::LogTypes type,
        std::shared_ptr<std::ostream> stream,
        LogLevels level) : 
    name_ (name),
    pimpl_(std::make_shared<LoggerData>())
{
    pimpl_->type_ = type;
    pimpl_->stream_ = stream;
    pimpl_->log_level_ = level;
}

// noop logger
VT::Logger::Logger(const std::string& name) :
    name_ (name),
    pimpl_(std::make_shared<LoggerData>())
{ }

VT::Logger::~Logger()
{ }

VT::Logger::Logger(const Logger& other) :
    name_ (other.name_),
    pimpl_(other.pimpl_)
{ }

VT::Logger& VT::Logger::operator=(const Logger& rhs)
{
    name_ = rhs.name_;
    pimpl_ = rhs.pimpl_;
    return *this;
}

VT::Logger& VT::Logger::set_opt(LogOpts options)
{
    pimpl_->default_opts_ |= options;
    return *this;
}

VT::Logger& VT::Logger::unset_opt(LogOpts options)
{
    pimpl_->default_opts_ &= ~options;
    return *this;
}

VT::Logger& VT::Logger::reset_opts()
{
    pimpl_->default_opts_ = LogOpt::Default;
    return *this;
}

void VT::Logger::disable_locking()
{
    pimpl_->use_lock_ = false;
}

void VT::Logger::enable_locking()
{
    pimpl_->use_lock_ = true;
}

VT::detail_::LogWorker VT::Logger::operator()(LogLevels level) const
{
    if (level < pimpl_->log_level_)
        return detail_::LogWorker(*detail_::dev_null_);

    CriticalSection* lock = (pimpl_->use_lock_ ? pimpl_->lock_.get() : NULL);

    switch (pimpl_->type_)
    {
    case detail_::LogType::Custom:
        assert(pimpl_->stream_);
        return detail_::LogWorker(*(pimpl_->stream_), lock, level, pimpl_->default_opts_);

    case detail_::LogType::Cout:
        return detail_::LogWorker(std::cout, lock, level, pimpl_->default_opts_);

    case detail_::LogType::Cerr:
        return detail_::LogWorker(std::cerr, lock, level, pimpl_->default_opts_);

    case detail_::LogType::Noop:
        return detail_::LogWorker(*detail_::dev_null_);

    default:
        assert(0);
        return detail_::LogWorker(*detail_::dev_null_);
    }
}


VT::Logger& VT::LogFactory::stream(const std::string& name,
                                   std::shared_ptr<std::ostream> stream,
                                   LogLevels level)
{
    return loggers_[name] = Logger(name, detail_::LogType::Custom, stream, level);
}

VT::Logger& VT::LogFactory::cout(const std::string& name,
                                 LogLevels level)
{
    return loggers_[name] = Logger(name,
                                   detail_::LogType::Cout,
                                   std::shared_ptr<std::ostream>(),
                                   level);
}

VT::Logger& VT::LogFactory::cerr(const std::string& name,
                                 LogLevels level)
{
    return loggers_[name] = Logger(name,
                                   detail_::LogType::Cerr,
                                   std::shared_ptr<std::ostream>(),
                                   level);
}

VT::Logger& VT::LogFactory::noop(const std::string& name)
{
    return loggers_[name] = Logger(name);
}

VT::Logger& VT::LogFactory::get(const std::string& name)
{
    auto it = loggers_.find(name);
    if (it == loggers_.end())
        throw std::runtime_error("Logger \"" + name + "\" does not exist");
    return it->second;
}
