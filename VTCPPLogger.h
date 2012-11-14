#pragma once

#include "VTCriticalSection.h"

#include <ostream>
#include <map>
#include <memory>
#include <string>


// TODO:
//   * implement coloring (separate implementations for each platform)
//   * log to multiple streams at the same time
//   * implement something similar to printf
//   * add custom timestamp formatting

/*
    Example:
        See VTCPPLoggerTest.cpp for example usage
*/

namespace VT
{
    // forward declarations
    class Logger;
    class LogFactory;
    

    // enumerations
    namespace LogLevel
    {
        enum LogLevels_
        {
            Debug       = 1,
            Info        = 2,
            Warning     = 3,
            Error       = 4,
            Critical    = 5
        };

    }
    typedef LogLevel::LogLevels_ LogLevels;


    namespace LogOpt
    {
        enum LogOpts_
        {
            Default     = 0u,
            NoSpace     = (1u << 0),
            NoEndl      = (1u << 1),
            NoPrefix    = (1u << 2),
            NoTimestamp = (1u << 3),
            NoFlush     = (1u << 4)
        };
    }
    typedef LogOpt::LogOpts_ LogOpts;


    // implementation details
    namespace detail_
    {
        namespace LogType
        {
            enum LogTypes_
            {
                Cout,
                Cerr,
                Custom,
                Noop
            };
        }
        typedef LogType::LogTypes_ LogTypes;


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


        class LogWorker
        {
        private:
            std::ostream& stream_;
            CriticalSection* lock_;     // if lock_ is NULL - don't use locking
            bool valid_;                // if not valid - this object has been moved
            LogLevels msg_level_;
            unsigned int options_;

        private:
            // helpers
            inline bool is_set(LogOpts opt) const;
            inline bool not_set(LogOpts opt) const;
            inline void set(LogOpts opt);

            // no copying allowed (use move)
            LogWorker(const LogWorker& other);
            LogWorker& operator=(const LogWorker& rhs);

        public:
            explicit
            LogWorker(std::ostream& stream,
                      CriticalSection* lock = NULL,
                      LogLevels msg_level = LogLevel::Debug,
                      unsigned int options = LogOpt::Default);

            // just in case RVO will not be used
            LogWorker(LogWorker&& other);

            // As soon as it goes out of scope it releases the lock and flushes output.
            // It should usually be created as temporary from Logger and so
            // it would go out of scope after the semicolon
            ~LogWorker();

            // setting options
            LogWorker& nospace();
            LogWorker& noendl();
            LogWorker& noprefix();
            LogWorker& notimestamp();

            // printers
            template <typename T>
            LogWorker& operator<<(T arg)
            {
                stream_ << arg;
                if (not_set(LogOpt::NoSpace))
                    stream_ << " ";
                return *this;
            }

            LogWorker& operator<<(std::ostream& (*manip)(std::ostream&));
            LogWorker& operator<<(std::ios_base& (*manip)(std::ios_base&));
        
            template<typename Arg1, typename... Args>
            inline void log(const Arg1& arg1, const Args&... args)
            {
                /*
                 * Recursive function to keep streaming the arguments 
                 * one at a time until the last argument is reached and 
                 * the specialization above is called. 
                 */
                (*this) << arg1;        // implement in terms of LogWorker::operator<<()
                log(args...);
            }

            /* A specialization to terminate the recursion.  */
            inline void log() const {  }
        };

    }

    
    class Logger
    {
    private:
        std::string name_;
        detail_::LogTypes type_;
        std::shared_ptr<std::ostream> stream_;
        std::shared_ptr<CriticalSection> lock_;
        bool use_lock_;
        LogLevels log_level_;
        unsigned int default_opts_;

    private:
        static std::shared_ptr<detail_::onullstream> dev_null_;

    private:
        // usual logger
        Logger( const std::string& name,
                detail_::LogTypes type,
                std::shared_ptr<std::ostream> stream,
                LogLevels level);

        // noop logger
        explicit
        Logger(const std::string& name);

        friend class LogFactory;

    public:
        // default constructed logger should not be used (here only because map requires it)
        Logger() { }

        Logger& set_opt(LogOpts options);
        Logger& unset_opt(LogOpts options);
        Logger& reset_opts();

        inline void disable_locking() { use_lock_ = false; }
        inline void enable_locking() { use_lock_ = true; }

        // use like this: logger() << "Hello " << std::hex << 10 << " world";
        // alternative syntax: logger().log("Hello ", std::hex, 10, " world");
        detail_::LogWorker operator()(LogLevels level = LogLevel::Debug) const;
    };
    

    // you should probably create and store a static instance of this class in your application
    class LogFactory
    {
    private:
        std::map<std::string, Logger> loggers_;

    public:
        /*
            Use    stream("logger_name", make_shared<std::ofstream>("filename"))   to create file logger
            or     cout("logger_name")                                             to create std::cout logger
            or     cerr("logger_name")                                             to create std::cerr logger
            or     noop("logger_name")                                             to create no-op logger

            Use    get("logger_name")    to retrieve existing logger
        */
        Logger& stream(const std::string& name,
                       std::shared_ptr<std::ostream> stream,
                       LogLevels level = LogLevel::Debug);

        Logger& cout(const std::string& name, LogLevels level = LogLevel::Debug);
        Logger& cerr(const std::string& name, LogLevels level = LogLevel::Debug);
        Logger& noop(const std::string& name);

        // get existing logger by name
        Logger& get(const std::string& name);
    };

}