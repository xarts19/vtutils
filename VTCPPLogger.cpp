#include "VTCPPLogger.h"

#include "VTThread.h"

#include <exception>
#include <iostream>
#include <iomanip>

#include <time.h>
#include <assert.h>

#define TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S"

// shut up fopen and localtime security warnings in Visual Studio
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4996)
#endif


std::string createTimestamp()
{
    time_t     now      = time( 0 );
    struct tm  timeinfo = *localtime( &now );
    char       buf[80];
    strftime( buf, sizeof( buf ), TIMESTAMP_FORMAT, &timeinfo );
    return buf;
}


VT::LogLevel VT::LogLevel_from_str(const std::string& level)
{
    if (level == "Debug")
        return LL_Debug;
    else if (level == "Info")
        return LL_Info;
    else if (level == "Warning")
        return LL_Warning;
    else if (level == "Error")
        return LL_Error;
    else if (level == "Critical")
        return LL_Critical;
    else
        return LL_NoLogging;
}

const char* getLogLevel(VT::LogLevel l)
{
    switch (l)
    {
    case VT::LL_Debug:
        return "Debug";
    case VT::LL_Info:
        return "Info";
    case VT::LL_Warning:
        return "Warning";
    case VT::LL_Error:
        return "Error";
    case VT::LL_Critical:
        return "Critical";
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
        cout_level  (LL_NoLogging),
        cerr_level  (LL_NoLogging),
        stream_level(LL_NoLogging),
        default_opts(LO_Default)
    { }

    std::string                    name;
    std::shared_ptr<std::FILE>     stream;
    LogLevel                       cout_level;
    LogLevel                       cerr_level;
    LogLevel                       stream_level;
    unsigned int                   default_opts;  // LogOpts flags
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
    assert(stream != nullptr && reporting_level != LL_NoLogging ||
           stream == nullptr && reporting_level == LL_NoLogging);

    if (stream)
    {
        pimpl_->stream = std::shared_ptr<std::FILE>(stream, std::fclose);
        pimpl_->stream_level = reporting_level;
    }
    else
    {
        pimpl_->stream = nullptr;
        pimpl_->stream_level = LL_NoLogging;
    }

    return true;
}

bool VT::Logger::set_stream(std::string filename, LogLevel reporting_level)
{
    assert(!filename.empty() && reporting_level != LL_NoLogging ||
           filename.empty() && reporting_level == LL_NoLogging);

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
    ::set(pimpl_->default_opts, opt);
}

void VT::Logger::unset(LogOpts opt)
{
    ::unset(pimpl_->default_opts, opt);
}

void VT::Logger::reset()
{
    pimpl_->default_opts = LO_Default;
}


VT::detail_::LogWorker VT::Logger::log(LogLevel level)
{
    return detail_::LogWorker(this, level, pimpl_->name, pimpl_->default_opts);
}
VT::detail_::LogWorker VT::Logger::debug()   { return log(LL_Debug); }
VT::detail_::LogWorker VT::Logger::info()    { return log(LL_Info); }
VT::detail_::LogWorker VT::Logger::warning() { return log(LL_Warning); }
VT::detail_::LogWorker VT::Logger::error()   { return log(LL_Error); }
VT::detail_::LogWorker VT::Logger::critical(){ return log(LL_Critical); }


void VT::Logger::log_worker(LogLevel level, const std::string& msg)
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



VT::detail_::LogWorker::LogWorker(Logger* logger, LogLevel level, const std::string& name, unsigned int opts)
    : logger_(logger)
    , msg_level_(level)
    , msg_stream_()
    , options_(opts)
{
    if (!is_set(opts, LO_NoTimestamp))
        msg_stream_ << createTimestamp() << " ";

    if (!is_set(opts, LO_NoLoggerName))
        msg_stream_ << "[" << name << "] ";

    std::ios::fmtflags f(msg_stream_.flags());

    if (!is_set(opts, LO_NoThreadId))
        msg_stream_ << "(0x" << std::hex << VT::Thread::current_thread_id() << ") ";

    msg_stream_.flags(f);  // restore state to undo std::hex changes to stream

    if (!is_set(opts, LO_NoLogLevel))
        msg_stream_ << std::left << std::setw(10) << "<" + std::string(getLogLevel(msg_level_)) + ">" << " ";
}

VT::detail_::LogWorker::~LogWorker()
{
    if (!is_set(options_, LO_NoEndl))
        msg_stream_ << std::endl;

    logger_->log_worker(msg_level_, msg_stream_.str());
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
