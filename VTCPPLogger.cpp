#include "VTCPPLogger.h"

#include <thread>
#include <mutex>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include <time.h>
#include <assert.h>

#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S"

#define LL_DEBUG    "Debug"
#define LL_INFO     "Info"
#define LL_WARNING  "Warning"
#define LL_ERROR    "Error"
#define LL_CRITICAL "Critical"

// shut up fopen and localtime security warnings in Visual Studio
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4996)
#endif


namespace VT
{
    class LogManager
    {
    private:
        friend VT::Logger get_logger(const std::string& name);
        friend void set_logger(const std::string& name, const Logger& logger);

        LogManager();
        ~LogManager();

        std::map<std::string, VT::Logger> loggers_;

        static LogManager self_;
        static bool self_valid_;

        std::mutex lock_;
    };
}


VT::LogManager VT::LogManager::self_;
bool VT::LogManager::self_valid_ = false;


std::string createTimestamp()
{
    time_t     now      = time(0);
    struct tm  timeinfo = *localtime(&now);
    char       buf[80];
    strftime(buf, sizeof(buf), TIMESTAMP_FORMAT, &timeinfo);
    return buf;
}


VT::LogLevel VT::LogLevel_from_str(const std::string& level)
{
    if (level == LL_DEBUG)
        return LL_Debug;
    else if (level == LL_INFO)
        return LL_Info;
    else if (level == LL_WARNING)
        return LL_Warning;
    else if (level == LL_ERROR)
        return LL_Error;
    else if (level == LL_CRITICAL)
        return LL_Critical;
    else
        return LL_NoLogging;
}

const char* getLogLevel(VT::LogLevel l)
{
    switch (l)
    {
    case VT::LL_Debug:
        return LL_DEBUG;
    case VT::LL_Info:
        return LL_INFO;
    case VT::LL_Warning:
        return LL_WARNING;
    case VT::LL_Error:
        return LL_ERROR;
    case VT::LL_Critical:
        return LL_CRITICAL;
    default:
        assert(0);
        return "Unknown";
    }
}


namespace
{
    bool is_set(unsigned int options, VT::LogOpts opt)
    {
        return (options & opt) == static_cast<unsigned int>(opt);
    }

    void set(unsigned int& options, VT::LogOpts opt)
    {
        options |= opt;
    }

    void unset(unsigned int& options, VT::LogOpts opt)
    {
        options &= ~opt;
    }
}



struct VT::Logger::Impl
{
    Impl(const std::string& name) :
        name        (name),
        stream      (nullptr),
        stream_lock (),
        cout_level  (LL_NoLogging),
        cerr_level  (LL_NoLogging),
        stream_level(LL_NoLogging),
        options     (LO_Default)
    { }

    Impl(const Impl& other) :
        name        (other.name),
        stream      (nullptr),
        stream_lock (),
        cout_level  (other.cout_level),
        cerr_level  (other.cerr_level),
        stream_level(other.stream_level),
        options     (other.options)
    {
        std::lock_guard<std::mutex> l(stream_lock);
        stream = other.stream;
    }

    ~Impl()
    {
        std::lock_guard<std::mutex> l(stream_lock);
        stream = nullptr;
    }

    std::string                    name;
    std::shared_ptr<std::FILE>     stream;
    std::mutex                     stream_lock;
    LogLevel                       cout_level;
    LogLevel                       cerr_level;
    LogLevel                       stream_level;
    unsigned int                   options;  // LogOpts flags
};


VT::Logger::Logger(const std::string& name)
    : pimpl_(new Impl(name))
{
}

VT::Logger::~Logger()
{
    delete pimpl_;
}

VT::Logger::Logger(const Logger& other)
    : pimpl_(new Impl(*other.pimpl_))
{
}

VT::Logger& VT::Logger::operator=(Logger other)
{
    swap(other);
    return *this;
}

void VT::Logger::swap(Logger& other)
{
    std::swap(pimpl_, other.pimpl_);
}


void VT::Logger::set_cout(LogLevel reporting_level)
{
    pimpl_->cout_level = reporting_level;
}

void VT::Logger::set_cerr(LogLevel reporting_level)
{
    pimpl_->cerr_level = reporting_level;
}

bool VT::Logger::set_stream(std::FILE* stream, LogLevel reporting_level)
{
    assert((stream != nullptr && reporting_level != LL_NoLogging) ||
           (stream == nullptr && reporting_level == LL_NoLogging));

    if (stream)
    {
        pimpl_->stream_level = reporting_level;
        std::lock_guard<std::mutex> lock(pimpl_->stream_lock);
        pimpl_->stream = std::shared_ptr<std::FILE>(stream, std::fclose);
    }
    else
    {
        pimpl_->stream_level = LL_NoLogging;
        std::lock_guard<std::mutex> lock(pimpl_->stream_lock);
        pimpl_->stream = nullptr;
    }

    return true;
}

bool VT::Logger::set_stream(const std::string& filename, LogLevel reporting_level)
{
    assert((!filename.empty() && reporting_level != LL_NoLogging) ||
           ( filename.empty() && reporting_level == LL_NoLogging));

    if (!filename.empty())
    {
        FILE* file = std::fopen(filename.c_str(), "a+");
        if (file)
            return set_stream(file, reporting_level);
        else
            return false;
    }
    else
    {
        return set_stream(nullptr, reporting_level);
    }
}


void VT::Logger::set(LogOpts opt)
{
    ::set(pimpl_->options, opt);
}

void VT::Logger::unset(LogOpts opt)
{
    ::unset(pimpl_->options, opt);
}

void VT::Logger::reset()
{
    pimpl_->options = LO_Default;
}


VT::detail_::LogWorker VT::Logger::log(LogLevel level)
{
    return detail_::LogWorker(this, level);
}
VT::detail_::LogWorker VT::Logger::debug()   { return log(LL_Debug); }
VT::detail_::LogWorker VT::Logger::info()    { return log(LL_Info); }
VT::detail_::LogWorker VT::Logger::warning() { return log(LL_Warning); }
VT::detail_::LogWorker VT::Logger::error()   { return log(LL_Error); }
VT::detail_::LogWorker VT::Logger::critical(){ return log(LL_Critical); }


void VT::Logger::add_prelude(std::string& out, LogLevel level)
{
    if (!is_set(pimpl_->options, LO_NoTimestamp))
        safe_sprintf(out, "{0} ", createTimestamp());
    if (!is_set(pimpl_->options, LO_NoLoggerName))
        safe_sprintf(out, "[{0}] ", pimpl_->name);
    if (!is_set(pimpl_->options, LO_NoThreadId))
        safe_sprintf(out, "0x{0:X} ", std::this_thread::get_id());
    if (!is_set(pimpl_->options, LO_NoLogLevel))
        safe_sprintf(out, "<{0}> ", getLogLevel(level));
}


void VT::Logger::add_epilog(std::string& out, LogLevel /*level*/)
{
    if (!is_set(pimpl_->options, LO_NoEndl))
        out.push_back('\n');
}


void VT::Logger::write_to_streams(LogLevel level, const std::string& msg)
{
    // fprintf is thread-safe on line level

    if (level >= pimpl_->cout_level)
    {
        fprintf(stdout, "%s", msg.c_str());
        fflush(stdout);
    }
    if (level >= pimpl_->cerr_level)
    {
        fprintf(stderr, "%s", msg.c_str());
        fflush(stderr);
    }
    if (level >= pimpl_->stream_level)
    {
        fprintf(pimpl_->stream.get(), "%s", msg.c_str());
        fflush(pimpl_->stream.get());
    }
}


void VT::quote(detail_::LogWorker& log_worker)
{
    log_worker.quote_ = true;
}


VT::Logger VT::get_logger(const std::string& name)
{
    if (!LogManager::self_valid_)
    {
        throw std::runtime_error("Trying to get logger after LogManager destruction");
    }

    std::lock_guard<std::mutex> lock(LogManager::self_.lock_);

    auto it = LogManager::self_.loggers_.find(name);
    if (it != LogManager::self_.loggers_.end())
        return it->second;

    auto pair = LogManager::self_.loggers_.insert(std::make_pair(name, Logger::cout(name)));
    return pair.first->second;
}


void VT::set_logger(const std::string& name, const Logger& logger)
{
    if (!LogManager::self_valid_)
    {
        throw std::runtime_error("Trying to get logger after LogManager destruction");
    }

    std::lock_guard<std::mutex> lock(LogManager::self_.lock_);

    auto it = LogManager::self_.loggers_.find(name);
    if (it != LogManager::self_.loggers_.end())
    {
        it->second = logger;
    }
    else
    {
        LogManager::self_.loggers_.insert(std::make_pair(name, logger));
    }
}


VT::LogManager::LogManager()
{
    self_valid_ = true;
}


VT::LogManager::~LogManager()
{
    self_valid_ = false;
}


VT::detail_::LogWorker::LogWorker(Logger* logger, LogLevel level)
    : logger_(logger)
    , msg_level_(level)
    , msg_stream_()
    , options_(logger_->pimpl_->options)
    , quote_(false)
{
    std::string out;
    logger_->add_prelude(out, level);
    msg_stream_ << out;
}


VT::detail_::LogWorker::~LogWorker()
{
    assert(logger_);
    std::string out(msg_stream_.str());
    logger_->add_epilog(out, msg_level_);
    logger_->write_to_streams(msg_level_, out);
}


VT::detail_::LogWorker::LogWorker(LogWorker&& other)
    : logger_(other.logger_)
    , msg_level_(other.msg_level_)
    , msg_stream_()
    , options_(other.options_)
    , quote_(other.quote_)
{
    // FIXME: use msg_stream_ move constructor to move it
    // as soon as gcc supports it
    msg_stream_ << other.msg_stream_.rdbuf();
    other.logger_ = nullptr;
}


void VT::detail_::LogWorker::optionally_add_space()
{
    if (!(options_ & LO_NoSpace))
        msg_stream_ << " ";
}


#ifdef _MSC_VER
    #pragma warning(pop)
#endif
