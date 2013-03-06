#pragma once

#include "VTCriticalSection.h"

#include <ostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cstdio>


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
        LL_Debug       = 1,
        LL_Info        = 2,
        LL_Warning     = 3,
        LL_Error       = 4,
        LL_Critical    = 5,
        LL_NoLogging   = 6
    };

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
        void set_stream(std::FILE* stream, LogLevel reporting_level = LL_Debug);


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
        friend detail_::LogWorker;


        // work function

        void log_worker(LogLevel level, const std::string& msg);
        

        // private data

        struct Impl;
        Impl* pimpl_;
    };


    namespace detail_
    {
        class LogWorker
        {
        public:
            LogWorker(Logger* logger, LogLevel level, const std::string& name, unsigned int opts);
            ~LogWorker();


            // printers

            template <typename T>
            LogWorker& operator<<(T arg)
            {
                msg_stream_ << arg;
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

        private:
            Logger*            logger_;
            LogLevel           msg_level_;
            std::ostringstream msg_stream_;
            unsigned int       options_;
        };
    }
}
