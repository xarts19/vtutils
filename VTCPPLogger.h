#pragma once

#include "VTCriticalSection.h"

#include <ostream>
#include <map>
#include <memory>
#include <string>
#include <vector>


// TODO:
//   * implement coloring (separate implementations for each platform)
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
        enum LogOpts_ : unsigned int
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


        struct LoggerData;

        
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
        extern std::shared_ptr<onullstream> dev_null_;
        

        class LogWorker
        {
        private:
            std::vector<LoggerData*> loggers_;
            bool valid_;                // if not valid - this object has been moved
            LogLevels msg_level_;
            unsigned int options_;

        private:
            // helpers
            inline bool is_set(LoggerData* l, LogOpts opt) const;
            inline bool not_set(LoggerData* l, LogOpts opt) const;
            inline void set(LogOpts opt);

            std::ostream& get_stream(LoggerData* data);

            // no copying allowed (use move)
            LogWorker(const LogWorker& other);
            LogWorker& operator=(const LogWorker& rhs);

        public:
            explicit
            LogWorker(std::vector<LoggerData*> loggers = std::vector<LoggerData*>(),
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
                for (LoggerData* l : loggers_)
                {
                    std::ostream& stream_ = get_stream(l);
                    stream_ << arg;
                    if (not_set(l, LogOpt::NoSpace))
                        stream_ << " ";
                }
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
        std::shared_ptr<detail_::LoggerData> pimpl_;

    private:
        // usual logger
        Logger( const std::string& name,
                detail_::LogTypes type,
                std::shared_ptr<std::ostream> stream,
                LogLevels level);

        // noop logger
        explicit
        Logger(const std::string& name);

        bool has_actual_stream() const;

        friend class LogFactory;
        friend class MetaLogger;

    public:
        // default constructed logger should not be used (here only because map requires it)
        Logger() { }
        ~Logger();

        Logger(const Logger& other);
        Logger& operator=(const Logger& rhs);

        Logger& set_opt(LogOpts options);
        Logger& unset_opt(LogOpts options);
        Logger& reset_opts();

        void disable_locking();
        void enable_locking();

        // use like this: logger() << "Hello " << std::hex << 10 << " world";
        // alternative syntax: logger().log("Hello ", std::hex, 10, " world");
        detail_::LogWorker operator()(LogLevels level = LogLevel::Debug) const;
    };
    

    class MetaLogger
    {
    private:
        std::string name_;
        std::vector<Logger> loggers_;

        friend LogFactory;

    private:
        MetaLogger(std::string name) : name_(name) { }

    public:
        MetaLogger() { }

        detail_::LogWorker operator()(LogLevels level = LogLevel::Debug) const;
    };


    // you should probably create and store a static instance of this class in your application
    class LogFactory
    {
    private:
        std::map<std::string, Logger> loggers_;
        std::map<std::string, MetaLogger> metaloggers_;

    private:
        void meta_helper(MetaLogger&) { }

        template <typename... Args>
        void meta_helper(MetaLogger& ml, std::string logname, Args... logger_names)
        {
            auto it = loggers_.find(logname);
            if (it == loggers_.end())
                throw std::runtime_error("Logger \"" + logname + "\" does not exist");
            ml.loggers_.push_back(it->second);

            meta_helper(ml, logger_names...);
        }


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

        
        template <typename... Args>
        MetaLogger& meta(std::string name, Args... logger_names)
        {
            MetaLogger ml(name);
            meta_helper(ml, logger_names...);
            return metaloggers_[name] = ml;
        }

        // get existing logger by name
        Logger& get(const std::string& name);
    };

}