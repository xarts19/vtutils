#include "VTCPPLogger.h"

#include <exception>
#include <iostream>

#include <time.h>
#include <assert.h>


// sink output into void
std::shared_ptr<VT::detail_::onullstream> VT::detail_::dev_null_ = 
    std::shared_ptr<VT::detail_::onullstream>(new VT::detail_::onullstream());


using VT::detail_::LoggerData;
using VT::detail_::LogWorker;
using VT::CriticalSection;
using VT::LogLevels;
using VT::LogOpts;
using std::string;
using std::ostream;
using std::vector;


namespace VT
{
    namespace detail_
    {
        struct LoggerData
        {
            LoggerData() :
                name_       (),
                type_       (LogType::Noop),
                stream_     (),
                lock_       (),
                use_lock_   (false),
                log_level_  (LogLevel::Critical),
                default_opts_(LogOpt::Default)
            { }

            std::string name_;
            VT::detail_::LogTypes type_;
            std::shared_ptr<ostream> stream_;
            std::shared_ptr<VT::CriticalSection> lock_;
            bool use_lock_;
            VT::LogLevels log_level_;
            unsigned int default_opts_;
        };
    }
}

string createTimestamp()
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
            vector<LoggerData*> loggers,
            LogLevels msg_level,
            unsigned int options) :
    loggers_(loggers),
    valid_(true),
    msg_level_(msg_level),
    options_(options)
{
    for (LoggerData* l : loggers_)
    {
        if (l->lock_ && l->use_lock_) l->lock_->enter();

        std::ostream& stream_ = get_stream(l);

        if (not_set(l, LogOpt::NoTimestamp))
            stream_ << createTimestamp() << " ";

        if (not_set(l, LogOpt::NoPrefix))
            stream_ << getLogLevel(msg_level_) << " ";
    }
}

LogWorker::LogWorker(LogWorker&& other) :
    loggers_(other.loggers_),
    valid_(true),
    msg_level_(other.msg_level_),
    options_(other.options_)
{
    // set so that other worker won't invoke usual destructor operations
    other.valid_ = false;
}

LogWorker::~LogWorker()
{
    if (valid_)
    {
        for (LoggerData* l : loggers_)
        {
            std::ostream& stream_ = get_stream(l);

            if (not_set(l, LogOpt::NoEndl))
                stream_ << std::endl;
            else if (not_set(l, LogOpt::NoFlush))
                stream_ << std::flush;

            if (l->lock_ && l->use_lock_) l->lock_->leave();
        }
    }
}

bool LogWorker::is_set(LoggerData* l, LogOpts opt) const
{
    return ( (options_ & opt) == opt || (l->default_opts_ & opt) == opt );
}

bool LogWorker::not_set(LoggerData* l, LogOpts opt) const
{
    return ( (options_ & opt) != opt && (l->default_opts_ & opt) != opt );
}

void LogWorker::set(LogOpts opt)
{
    options_ |= opt;
}

ostream&
    LogWorker::get_stream(LoggerData* data)
{
    switch (data->type_)
    {
    case LogType::Custom:
        return *(data->stream_);
    case LogType::Cout:
        return std::cout;
    case LogType::Cerr:
        return std::cerr;
    default:
        return *dev_null_;
    }
}

LogWorker&
    VT::detail_::LogWorker::nospace()
{
    set(LogOpt::NoSpace);
    return *this;
}

LogWorker&
    VT::detail_::LogWorker::noendl()
{
    set(LogOpt::NoEndl);
    return *this;
}

LogWorker&
    VT::detail_::LogWorker::noprefix()
{
    set(LogOpt::NoPrefix);
    return *this;
}

LogWorker&
    VT::detail_::LogWorker::notimestamp()
{
    set(LogOpt::NoTimestamp);
    return *this;
}

LogWorker&
    VT::detail_::LogWorker::operator<<(ostream& (*manip)(ostream&))
{
    for (LoggerData* l : loggers_)
    {
        manip(get_stream(l));
    }
	return *this;
}

LogWorker&
    VT::detail_::LogWorker::operator<<(std::ios_base& (*manip)(std::ios_base&))
{
    for (LoggerData* l : loggers_)
    {
        manip(get_stream(l));
    }
	return *this;
}


// usual logger
VT::Logger::Logger(
        const string& name,
        detail_::LogTypes type,
        std::shared_ptr<ostream> stream,
        LogLevels level) : 
    pimpl_(std::make_shared<LoggerData>())
{
    pimpl_->name_ = name;
    pimpl_->type_ = type;
    pimpl_->stream_ = stream;
    pimpl_->log_level_ = level;
}

// noop logger
VT::Logger::Logger(const string& name) :
    pimpl_(std::make_shared<LoggerData>())
{
    pimpl_->name_ = name;
}

bool VT::Logger::has_actual_stream() const
{
    return (pimpl_->type_ == detail_::LogType::Custom ||
            pimpl_->type_ == detail_::LogType::Cout ||
            pimpl_->type_ == detail_::LogType::Cerr);
}

VT::Logger::~Logger()
{ }

VT::Logger::Logger(const Logger& other) :
    pimpl_(other.pimpl_)
{ }

VT::Logger& VT::Logger::operator=(const Logger& rhs)
{
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

LogWorker VT::Logger::operator()(LogLevels level) const
{
    if (level < pimpl_->log_level_)
        return detail_::LogWorker();

    if (!has_actual_stream())
    {
        return detail_::LogWorker();
    }

    vector<LoggerData*> ls;
    ls.push_back(pimpl_.get());

    return detail_::LogWorker(ls, level, pimpl_->default_opts_);
}


LogWorker VT::MetaLogger::operator()(LogLevels level) const
{
    vector<LoggerData*> ls;
    
    for (auto logger : loggers_)
    {
        if (level >= logger.pimpl_->log_level_ && logger.has_actual_stream())
            ls.push_back(logger.pimpl_.get());
    }

    return detail_::LogWorker(ls, level, LogOpt::Default);
}


VT::Logger& VT::LogFactory::stream(const string& name,
                                   std::shared_ptr<ostream> stream,
                                   LogLevels level)
{
    return loggers_[name] = Logger(name, detail_::LogType::Custom, stream, level);
}

VT::Logger& VT::LogFactory::cout(const string& name,
                                 LogLevels level)
{
    return loggers_[name] = Logger(name,
                                   detail_::LogType::Cout,
                                   std::shared_ptr<ostream>(),
                                   level);
}

VT::Logger& VT::LogFactory::cerr(const string& name,
                                 LogLevels level)
{
    return loggers_[name] = Logger(name,
                                   detail_::LogType::Cerr,
                                   std::shared_ptr<ostream>(),
                                   level);
}

VT::Logger& VT::LogFactory::noop(const string& name)
{
    return loggers_[name] = Logger(name);
}

VT::Logger& VT::LogFactory::get(const string& name)
{
    auto it = loggers_.find(name);
    if (it == loggers_.end())
        throw std::runtime_error("Logger \"" + name + "\" does not exist");
    return it->second;
}
