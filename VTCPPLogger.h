#pragma once

#include "VTUtil.h"

#include <ostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdio>


// define this macro if you don't want to use VTThread (you won't see thread id)
//#define VT_LOGGER_DONT_USE_VTTHREAD

// TODO:
//   * implement coloring (separate implementations for each platform)
//   * add custom timestamp formatting
/*
    Example:
        See VTCPPLoggerTest.cpp for example usage
*/

namespace VT
{
    // forward declarations
    namespace detail_ { class LogWorker; }
    

    // enumerations

    enum LogLevel
    {
        LL_Debug       = 0,
        LL_Info        = 1,
        LL_Warning     = 2,
        LL_Error       = 3,
        LL_Critical    = 4,
        LL_NoLogging   = 5
    };

    // return LL_NoLogging on unknown strings
    LogLevel LogLevel_from_str(const std::string& level);

    enum LogOpts
    {
        LO_Default     = 0u,
        LO_NoEndl      = (1u << 0),
        LO_NoLogLevel  = (1u << 1),
        LO_NoTimestamp = (1u << 2),
        LO_NoFlush     = (1u << 3),
        LO_NoLoggerName= (1u << 4),
        LO_NoThreadId  = (1u << 5),
        LO_NoSpace     = (1u << 6)
    };


    // logging functions should be thread-safe
    // but anything that changes the logger is probably not
    // logger is copyable and copy will share file stream of
    // original logger
    class Logger
    {
    public:
        Logger(const std::string& name);
        ~Logger();

        Logger(const Logger& other);
        Logger& operator=(Logger other);

        void swap(Logger& other);


        // convenience methods

        static Logger cout(const std::string& name, LogLevel reporting_level = LL_Debug)
        {
            Logger l(name);
            l.set_cout(reporting_level);
            return l;
        }

        static Logger cerr(const std::string& name, LogLevel reporting_level = LL_Debug)
        {
            Logger l(name);
            l.set_cerr(reporting_level);
            return l;
        }

        static Logger stream(const std::string& name,
                             const std::string& filename,
                             LogLevel reporting_level = LL_Debug)
        {
            Logger l(name);
            l.set_stream(filename, reporting_level);
            return l;
        }

        static Logger stream(const std::string& name,
                             std::FILE* stream,
                             LogLevel reporting_level = LL_Debug)
        {
            Logger l(name);
            l.set_stream(stream, reporting_level);
            return l;
        }


        // enable certain streams
        // passing LL_NoLogging desables the stream
        // pass nullptr as stream in set_stream when passing LL_NoLogging

        void set_cout(LogLevel reporting_level = LL_Debug);
        void set_cerr(LogLevel reporting_level = LL_Debug);

        // logger assumes control of FILE object and closes it when
        // all copies of this logger are destroyed or have new stream set
        bool set_stream(std::FILE* stream, LogLevel reporting_level = LL_Debug);

        // returns false when failed to open the file
        // uses std::fopen, so use apropriate functions to get error codes
        bool set_stream(const std::string& filename, LogLevel reporting_level = LL_Debug);


        // modify logger options

        void set(LogOpts opt);
        void unset(LogOpts opt);
        void reset();


        // returns temprorary object for atomic write

        detail_::LogWorker log(LogLevel level);

        detail_::LogWorker debug();
        detail_::LogWorker info();
        detail_::LogWorker warning();
        detail_::LogWorker error();
        detail_::LogWorker critical();

    private:
        friend class detail_::LogWorker;


        // work function

        void log_worker(LogLevel level, const std::string& msg);
        

        // private data

        struct Impl;
        Impl* pimpl_;
    };


    // stream manipulator to surround next argument with quotes
    void quote(detail_::LogWorker& log_worker);
    inline const char* yes_no(bool flag) { return (flag ? "yes" : "no"); }


    namespace detail_
    {
        class LogWorker
        {
        public:
            LogWorker(Logger* logger, LogLevel level, const std::string& name, unsigned int opts);
            ~LogWorker();
            
#ifdef __GNUC__
            LogWorker(LogWorker&& other);
#endif
            

            // printers

            template <typename T>
            LogWorker& operator<<(T arg)
            {
                if (quote_)
                    msg_stream_ << '"';

                msg_stream_ << arg;

                if (quote_)
                {
                    msg_stream_ << '"';
                    quote_ = false;
                }

                if (!(options_ & LO_NoSpace))
                    msg_stream_ << " ";
                return *this;
            }

            LogWorker& operator<<(std::ostream& (*manip)(std::ostream&))
            {
                manip(msg_stream_);
                return *this;
            }

            LogWorker& operator<<(std::ios_base& (*manip)(std::ios_base&))
            {
                manip(msg_stream_);
                return *this;
            }

            LogWorker& operator<<(void (*manip)(LogWorker&))
            {
                manip(*this);
                return *this;
            }

        private:
            // deleted
            LogWorker& operator=(const LogWorker&);

            friend void VT::quote(LogWorker& log_worker);
            
            Logger*            logger_;
            LogLevel           msg_level_;
            std::ostringstream msg_stream_;
            unsigned int       options_;
            bool               quote_;
        };
    }
}
