#pragma once

#include "VTCriticalSection.h"

#include <iostream>
#include <fstream>
#include <ostream>
#include <map>
#include <memory>
#include <string>
#include <exception>

#include <assert.h>
#include <time.h>

// TODO:
//   * implement coloring (separate implementations for each platform)
//   * log to multiple streams at the same time
//   * implement something similar to printf
//   * add custom timestamp formatting
//   * move to cpp

/*
    Example:
        See VTCPPLoggerTest.cpp for example usage
*/

namespace VT
{
    // forward declareations
    class Logger;
    class LogFactory;
    

    // enumerations

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
    typedef LogLevel::LogLevels LogLevels;

    namespace LogOpt
    {
        enum LogOpts
        {
            Default     = 0u,
            NoSpace     = (1u << 0),
            NoEndl      = (1u << 1),
            NoPrefix    = (1u << 2),
            NoTimestamp = (1u << 3),
            NoFlush     = (1u << 4)
        };
    }
    typedef LogOpt::LogOpts LogOpts;


    // implementation details
    namespace detail_
    {
        namespace LogType
        {
            enum LogTypes
            {
                Cout,
                Cerr,
                Custom,
                Noop
            };
        }
        typedef LogType::LogTypes LogTypes;


        inline const char* getLogLevel(LogLevels l)
        {
            switch (l)
            {
            case LogLevels::Debug:
                return "Debug   ";
            case LogLevels::Info:
                return "Info    ";
            case LogLevels::Warning:
                return "Warning ";
            case LogLevels::Error:
                return "Error   ";
            case LogLevels::Critical:
                return "Critical";
            default:
                return "Unknown ";
            }
        }


        inline std::string createTimestamp()
        {
            time_t     now      = time( 0 );
            struct tm  timeinfo = *localtime( &now );
            char       buf[80];
            strftime( buf, sizeof( buf ), "%d/%m/%y %H:%M:%S", &timeinfo );
            return buf;
        }


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
            // helper
            inline bool is_set(LogOpts opt) const
            {
                return ( (options_ & opt) == opt );
            }
            inline bool not_set(LogOpts opt) const
            {
                return ( (options_ & opt) != opt );
            }
            inline void set(LogOpts opt)
            {
                options_ |= opt;
            }

            LogWorker(const LogWorker& other);
            LogWorker& operator=(const LogWorker& rhs);

        public:

            // don't plan any funny business
            // users can't create LogWorkers
            // and besides, it's defined in detail_ namespace, so hands off!!!
            explicit
            LogWorker(std::ostream& stream,
                      CriticalSection* lock = NULL,
                      LogLevels msg_level = LogLevel::Debug,
                      unsigned int options = LogOpt::Default) :
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

            // just in case RVO will not be used
            LogWorker(LogWorker&& other) :
                stream_(other.stream_),
                lock_(other.lock_),
                valid_(true)
            {
                // set so that other worker won't invoke usual destructor operations
                other.valid_ = false;
            }

            // As soon as it goes out of scope it releases the lock and flushes output.
            // It should usually be created as temporary from Logger and so
            // it would go out of scope after the semicolon
            ~LogWorker()
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

            // setting options
            LogWorker& nospace()
            {
                set(LogOpt::NoSpace);
                return *this;
            }

            LogWorker& noendl()
            {
                set(LogOpt::NoEndl);
                return *this;
            }

            LogWorker& noprefix()
            {
                set(LogOpt::NoPrefix);
                return *this;
            }

            LogWorker& notimestamp()
            {
                set(LogOpt::NoTimestamp);
                return *this;
            }

            // printers
            template <typename T>
            LogWorker& operator<<(T arg)
            {
                stream_ << arg;
                if (not_set(LogOpt::NoSpace))
                    stream_ << " ";
                return *this;
            }

            LogWorker& operator<<(std::ostream& (*manip)(std::ostream&))
            {
                manip(stream_);
	            return *this;
            }

            LogWorker& operator<<(std::ios_base& (*manip)(std::ios_base&))
            {
                manip(stream_);
	            return *this;
            }
        
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

            inline void log()
            { /* A specialization to terminate the recursion.  */ }

        private:
            std::ostream& stream_;
            CriticalSection* lock_;     // if lock_ is NULL - don't use locking
            bool valid_;
            LogLevels msg_level_;
            unsigned int options_;
        };

    }

    
    class Logger
    {
    private:
        // usual logger
        Logger( const std::string& name,
                detail_::LogTypes type,
                std::shared_ptr<std::ostream> stream,
                LogLevels level) : 
            name_(name),
            type_(type),
            stream_(stream),
            lock_(std::make_shared<CriticalSection>()),
            use_lock_(true),
            log_level_(level),
            default_opts_(LogOpt::Default)
        { }

        // noop logger
        explicit
        Logger(const std::string& name) :
            name_(name),
            type_(detail_::LogType::Noop),
            stream_(dev_null_),
            lock_(),
            use_lock_(false),
            log_level_(),
            default_opts_(LogOpt::Default)
        { }

        friend class LogFactory;

    public:
        // default constructed logger should not be used (here only because map requires it)
        Logger() { }

        Logger& set_opt(LogOpts options)
        {
            default_opts_ |= options;
            return *this;
        }

        Logger& unset_opt(LogOpts options)
        {
            default_opts_ &= ~options;
            return *this;
        }

        Logger& reset_opts()
        {
            default_opts_ = LogOpt::Default;
            return *this;
        }

        void disable_locking()
        {
            use_lock_ = false;
        }

        void enable_locking()
        {
            use_lock_ = true;
        }

        // use like this: logger() << "Hello " << std::hex << 10 << " world";
        // alternative syntax: logger().log("Hello ", std::hex, 10, " world");
        detail_::LogWorker operator()(LogLevels level = LogLevel::Debug) const
        {
            if (level < log_level_)
                return detail_::LogWorker(*dev_null_);

            CriticalSection* lock = (use_lock_ ? lock_.get() : NULL);

            switch (type_)
            {
            case detail_::LogType::Custom:
                assert(stream_);
                return detail_::LogWorker(*stream_, lock, level, default_opts_);

            case detail_::LogType::Cout:
                return detail_::LogWorker(std::cout, lock, level, default_opts_);

            case detail_::LogType::Cerr:
                return detail_::LogWorker(std::cerr, lock, level, default_opts_);

            case detail_::LogType::Noop:
                return detail_::LogWorker(*dev_null_);

            default:
                assert(0);
                return detail_::LogWorker(*dev_null_);
            }
        }
    
    private:
        std::string name_;
        detail_::LogTypes type_;
        std::shared_ptr<std::ostream> stream_;
        std::shared_ptr<CriticalSection> lock_;
        bool use_lock_;
        LogLevels log_level_;
        unsigned int default_opts_;

        static std::shared_ptr<detail_::onullstream> dev_null_;
    };
    

    // you should probably create and store a static instance of this class in your application
    class LogFactory
    {
    public:
        /*
            Use    stream("logger_name", make_shared<std::ofstream>("filename"))   to create file logger
            or     cout("logger_name")                                             to create std::cout logger
            or     cerr("logger_name")                                             to create std::cerr logger
            or     noop("logger_name")                                             to create no-op logger

            Use    get("logger_name")    to retrieve existing logger
        */

        Logger& stream(const std::string& name, std::shared_ptr<std::ostream> stream, LogLevels level = LogLevel::Debug)
        {
            return loggers_[name] = Logger(name, detail_::LogType::Custom, stream, level);
        }

        Logger& cout(const std::string& name, LogLevels level = LogLevel::Debug)
        {
            return loggers_[name] = Logger(name, detail_::LogType::Cout, std::shared_ptr<std::ostream>(), level);
        }

        Logger& cerr(const std::string& name, LogLevels level = LogLevel::Debug)
        {
            return loggers_[name] = Logger(name, detail_::LogType::Cerr, std::shared_ptr<std::ostream>(), level);
        }

        Logger& noop(const std::string& name)
        {
            return loggers_[name] = Logger(name);
        }

        // get existing logger by name
        Logger& get(const std::string& name)
        {
            auto it = loggers_.find(name);
            if (it == loggers_.end())
                throw std::runtime_error("Logger \"" + name + "\" does not exist");
            return it->second;
        }
        
    private:
        std::map<std::string, Logger> loggers_;
    };

}