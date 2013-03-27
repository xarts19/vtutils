#ifdef _MSC_VER
#pragma warning( disable : 4996 )
#endif
#include "VTLogger.h"

#include <time.h>

namespace
{
    std::set<std::string> default_sinks()
    {
        std::set<std::string> sinks;
        return sinks;
    }
}

bool VT::Logger::prepend_time = false;
bool VT::Logger::use_stdout = false;
VT::Logger::LogLevel VT::Logger::log_level = VT::Logger::Warning;

VT::Lock VT::Logger::lock;
std::set<std::string>  VT::Logger::sinks = default_sinks();

std::string VT::current_date_time()
{
    time_t     now = time( 0 );
    struct tm  timeinfo;
    char       buf[80];
    timeinfo = *localtime( &now );
    strftime( buf, sizeof( buf ), "%d/%m/%y %H:%M:%S", &timeinfo );
    return buf;
}

void VT::Logger::register_file( std::string filename )
{
    sinks.insert( filename );
}

void VT::Logger::logger_worker( std::string message )
{
    lock.enter();

    for ( std::set<std::string>::const_iterator it = sinks.begin(); it != sinks.end(); ++it )
    {
        FILE* f = fopen( it->c_str(), "a+" );

        if ( f )
        {
            fprintf( f, "%s", message.c_str() );
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
