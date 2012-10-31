#pragma once
#pragma warning( disable : 4996 )

#include "VTCriticalSection.h"

#include <string>
#include <set>
#include <stdio.h>
#include <stdarg.h>

namespace VT
{

    std::string current_date_time();

    class Logger
    {
    public:
        enum LogLevel { Empty = 0, Critical = 1, Warning = 2, Debug = 3 };

        Logger( bool time = false, bool u_stdout = false ) :
            saved_prepend_time( prepend_time ),
            saved_use_stdout( use_stdout )
        {
            prepend_time = time;
            use_stdout = u_stdout;
        }

        ~Logger()
        {
            prepend_time = saved_prepend_time;
            use_stdout = saved_use_stdout;
        }

        template<LogLevel msg_loglevel>
        static void log( const char* fmt, ... )
        {
            if ( msg_loglevel > log_level )
                return;

            std::string message;
            va_list ap;

            char buf[1024];

            va_start( ap, fmt );
            int len = vsnprintf( buf, 1024, fmt, ap );
            va_end( ap );
            if (len > 1024)
                printf("Failed to print whole message\n");

            message = buf;

            if ( prepend_time )
                message = current_date_time() + " " + message;

            switch ( msg_loglevel )
            {
                case Debug:
                    message = "DEBUG    " + message;
                    break;

                case Warning:
                    message = "WARNING  " + message;
                    break;

                case Critical:
                    message = "CRITICAL " + message;
                    break;

                case Empty:
                    break;
            }

            logger_worker( message );
        }

        static void register_file( std::string filename );

    public:
        bool saved_prepend_time;
        bool saved_use_stdout;

    public:
        static bool prepend_time;
        static bool use_stdout;
        static LogLevel log_level;

    private:
        static void logger_worker( std::string message );

    private:
        static CriticalSection lock;
        static std::set<std::string> sinks;
    };
};