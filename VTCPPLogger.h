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

// TODO:
//   * implement log levels
//      * if log level < set log level => instantiate LogWorker with nullstream
//   * implement coloring (separate implementations for each platform)
//   * log to multiple streams at the same time
//   * pass options to log factory as bitflags (no space, no endl, no prefix, no timestamp)
//   * implement something similar to printf
//   * add prefix / timestamp
//   * add flag to disable locking

/*

    Example:
        VT::LogFactory log_factory;

        VT::Logger& logger1 = log_factory.stream("main_cout", std::make_shared<std::ofstream>("1.txt"));
        VT::Logger& logger2 = log_factory.cout("main");

        // same as logger2
        VT::Logger& logger3 = log_factory.get("main");

        // logger automatically appends std::endl after last token and flushes output
        // as well as adds spaces between tokens

        logger1() << "Hello " << 55 << " done";
        logger1() << std::hex << 11 << " done";

        logger1().log("smth", " is ", 25, " times ", std::hex, 12, " wrong");
        logger1().log();
        logger1().log("Nothing");

        VT::Logger& logger4 = log_factory.noop("noop");
        logger4() << "You should not see this";

*/

namespace VT
{
    // forward declareations
    class Logger;
    class LogFactory;

    // implementation details
    namespace detail_
    {
        class LogWorker;
    }

    
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
            Default = 0x00,
            NoSpace = 0x01,
            NoEndl  = 0x02
        };
    }
    typedef LogOpt::LogOpts LogOpts;

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


        class LogWorker
        {
        private:

            // don't plan any funny business
            // users can't create LogWorkers
            // and besides, it's defined in detail_ namespace, so hands off!!!
            explicit
            LogWorker(std::ostream& stream, CriticalSection* lock = NULL) : stream_(stream), lock_(lock), valid_(true)
            {
                if (lock_) lock_->enter();
            }
        
            LogWorker(const LogWorker& other);
            LogWorker& operator=(const LogWorker& rhs);

            friend class Logger;

        public:

            // just in case RVO will not be used
            LogWorker(LogWorker&& other) : stream_(other.stream_), lock_(other.lock_), valid_(true)
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
                    *this << std::endl;
                    if (lock_) lock_->leave();
                }
            }

            template <typename T>
            LogWorker& operator<<(T arg)
            {
                stream_ << arg << " ";
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
            {
                /*
                 * A specialization to terminate the recursion. 
                 */
            }

        private:
            std::ostream& stream_;
            CriticalSection* lock_;     // if lock_ is NULL - don't use locking
            bool valid_;
        };

    }

    
    class Logger
    {
    private:
        Logger(const std::string& name, detail_::LogTypes type, std::shared_ptr<std::ostream> stream) : 
            name_(name),
            type_(type),
            stream_(stream),
            lock_(std::make_shared<CriticalSection>())
        { }

        explicit
        Logger(const std::string& name) :
            name_(name),
            type_(detail_::LogType::Noop),
            stream_(dev_null_),
            lock_()
        { }

        friend class LogFactory;

    public:
        Logger() :
            name_(""),
            type_(detail_::LogType::Noop),
            stream_(dev_null_),
            lock_()
        { }

        Logger(const Logger& other)
        {
            name_ = other.name_;
            type_ = other.type_;
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
            swap(first.type_, second.type_);
            swap(first.stream_, second.stream_);
            swap(first.lock_, second.lock_);
        }

        // use like this: logger() << "Hello " << std::hex << 10 << " world";
        // alternative syntax: logger().log("Hello ", std::hex, 10, " world");
        detail_::LogWorker operator()() const
        {
            switch (type_)
            {
            case detail_::LogType::Custom:
                assert(stream_);
                return detail_::LogWorker(*stream_, lock_.get());

            case detail_::LogType::Cout:
                return detail_::LogWorker(std::cout, lock_.get());

            case detail_::LogType::Cerr:
                return detail_::LogWorker(std::cerr, lock_.get());

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

        const Logger& stream(const std::string& name, std::shared_ptr<std::ostream> stream)
        {
            return loggers_[name] = Logger(name, detail_::LogType::Custom, stream);
        }

        const Logger& cout(const std::string& name)
        {
            return loggers_[name] = Logger(name, detail_::LogType::Cout, std::shared_ptr<std::ostream>());
        }

        const Logger& cerr(const std::string& name)
        {
            return loggers_[name] = Logger(name, detail_::LogType::Cerr, std::shared_ptr<std::ostream>());
        }

        const Logger& noop(const std::string& name)
        {
            return loggers_[name] = Logger();
        }

        // get existing logger by name
        const Logger& get(const std::string& name) const
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