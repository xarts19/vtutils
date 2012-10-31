#pragma warning( disable : 4996 )
#include "VTLogger.h"

#include <time.h>

namespace
{
    std::set<std::string> default_sinks()
    {
        std::set<std::string> sinks;
        return sinks;
    }
};

bool Logger::prepend_time = false;
bool Logger::use_stdout = false;
Logger::LogLevel Logger::log_level = Logger::Debug;

VTCriticalSection Logger::lock;
std::set<std::string>  Logger::sinks = default_sinks();

std::string current_date_time()
{
    time_t     now = time( 0 );
    struct tm  timeinfo;
    char       buf[80];
    timeinfo = *localtime( &now );
    strftime( buf, sizeof( buf ), "%d/%m/%y %H:%M:%S", &timeinfo );
    return buf;
}

void Logger::register_file( std::string filename )
{
    sinks.insert( filename );
}

void Logger::logger_worker( std::string message )
{
    lock.enter();

    for ( std::set<std::string>::const_iterator it = sinks.begin(); it != sinks.end(); ++it )
    {
        FILE* f = fopen( it->c_str(), "a+" );

        if ( f )
        {
            fputs( message.c_str(), f );
            fflush( f );
            fclose( f );
        }
    }

    if ( use_stdout )
    {
        printf( "%s", message.c_str() );
    }

    lock.leave();
}