#pragma once

#include <iostream>
#include <fstream>
#include <ostream>
#include <map>
#include <memory>
#include <string>

#include "VTCriticalSection.h"

// TODO:
//   * function to return noop logger
//   * implement log levels
//      * if log level < set log level => instantiate LoggerWorker with nullstream
//   * implement coloring (separate implementations for each platform)
//   * log to multiple streams at the same time
//   * log to cerr

namespace VT
{
    // forward declareations
    class Logger;
    class LoggerWorker;
    class LogFactory;

    namespace LogLevel
    {
        enum LogLevels
        {
            Debug       = 1,
            Info        = 2,
            Warning     = 3,
            Error       = 4,
            Critical    = 5
        };

    }

    // implementation details
    namespace detail_
    {

        template <class cT, class traits = std::char_traits<cT> >
        class basic_nullbuf: public std::basic_streambuf<cT, traits> {
            typename traits::int_type overflow(typename traits::int_type c)
            {
                return traits::not_eof(c); // indicate success
            }
        };


        template <class cT, class traits = std::char_traits<cT> >
        class basic_onullstream: public std::basic_ostream<cT, traits> {
            public:
                basic_onullstream():
                std::basic_ios<cT, traits>(&m_sbuf),
                std::basic_ostream<cT, traits>(&m_sbuf)
                {
                    init(&m_sbuf);
                }

            private:
                basic_nullbuf<cT, traits> m_sbuf;
        };


        typedef basic_onullstream<char> onullstream;
        typedef basic_onullstream<wchar_t> wonullstream;


        class LoggerWorker
        {
        public:
            ~LoggerWorker()
            {
                *this << std::endl;
                lock_.leave();
            }

            template <typename T>
            LoggerWorker& operator<<(T arg)
            {
                stream_ << arg;
                return *this;
            }

            LoggerWorker& operator<<(std::ostream& (*manip)(std::ostream&))
            {
                manip(stream_);
	            return *this;
            }

            LoggerWorker& operator<<(std::ios_base& (*manip)(std::ios_base&))
            {
                manip(stream_);
	            return *this;
            }

            template <typename Arg1>
            void log(const Arg1& arg1)
            {
                /*
                 * A specialization to stream the last argument 
                 * and terminate the recursion. 
                 */
                stream_ << arg1 << std::endl;
            }
        
            template<typename Arg1, typename... Args>
            void log(const Arg1& arg1, const Args&... args)
            {
                /*
                 * Recursive function to keep streaming the arguments 
                 * one at a time until the last argument is reached and 
                 * the specialization above is called. 
                 */
                stream_ << arg1;
                log(args...);
            }

        private:
            LoggerWorker(std::ostream& stream, CriticalSection& lock) : stream_(stream), lock_(lock)
            {
                lock.enter();
            }
        
            // don't plan any funny business
            LoggerWorker(const LoggerWorker& other);
            LoggerWorker& operator=(const LoggerWorker& rhs);

            friend class Logger;

        private:
            std::ostream& stream_;
            CriticalSection& lock_;
        };

    }

    
    class Logger
    {
    public:
        Logger() : name_(""), stream_() { }
        
        Logger(std::string name, std::shared_ptr<std::ostream> stream) : name_(name), stream_(stream), lock_(std::make_shared<CriticalSection>()) { }
        
        Logger(const Logger& other)
        {
            name_ = other.name_;
            stream_ = other.stream_;
            lock_ = other.lock_;
        }
        
        Logger& operator=(Logger rhs)
        {
            swap(*this, rhs);
            return *this;
        }
        
        friend void swap(Logger& first, Logger& second)
        {
            using std::swap;
            swap(first.name_, second.name_);
            swap(first.stream_, second.stream_);
            swap(first.lock_, second.lock_);
        }

        // use like this: logger() << "Hello " << std::hex << 10 << " world";
        detail_::LoggerWorker operator()()
        {
            if (stream_)
                return detail_::LoggerWorker(*stream_, *lock_);
            else
                return detail_::LoggerWorker(std::cout, *lock_);
        }

        // alternative syntax: logger.log("Hello ", std::hex, 10, " world");
        template<typename... Args>
        void log(const Args&... args)
        {
            if (stream_)
                return detail_::LoggerWorker(*stream_, *lock_).log(args...);
            else
                return detail_::LoggerWorker(std::cout, *lock_).log(args...);
        }
    
    private:
        std::string name_;
        std::shared_ptr<std::ostream> stream_;
        std::shared_ptr<CriticalSection> lock_;
    };
    

    class LogFactory
    {
    public:
        /*
            Use    get("logger_name", make_shared<std::ofstream>("filename"))   to create file logger
            or     get("logger_name")                                           to create std::cout logger
        */
        static Logger& get(const std::string& name, std::shared_ptr<std::ostream> stream = std::shared_ptr<std::ostream>())
        {
            return loggers_[name] = Logger(name, stream);
        }
        
    private:
        static std::map<std::string, Logger> loggers_;
    };

}