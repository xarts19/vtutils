#include "VTJson_parser.h"

#include "../debug.h"

#include <sstream>
#include <iostream>
#include <iomanip>

std::string Json_Parser::insert_escapes( const std::string& str )
{
    std::string result = str;

    const char* to_escape = "\"\\/\b\f\n\r\t";

    size_t pos = str.find_first_of( to_escape, 0 );

    while ( pos != str.npos )
    {
        result.insert( pos, 1, '\\' );
        pos = str.find_first_of( to_escape, pos + 2 );
    }

    return result;
}

std::string Json_Parser::bytes_to_string( unsigned long long bytes )
{
    std::stringstream str;
    str << std::fixed;

    if ( bytes > 1024 * 1024 * 1024 )
    {
        str << std::setprecision( 2 ) << bytes / double( 1024 * 1024 * 1024 ) << " GB";
    }
    else if ( bytes > 1024 * 1024 )
    {
        str << std::setprecision( 2 ) << bytes / double( 1024 * 1024 ) << " MB";
    }
    else if ( bytes > 1024 )
    {
        str << bytes / 1024 << " KB";
    }
    else
    {
        str << bytes << " B";
    }

    return str.str();
}


Json_Value* Json_Value::create() { return new Json_Null(); }
Json_Value* Json_Value::create( bool value ) { return new Json_Bool( value ); }
Json_Value* Json_Value::create( long value ) { return new Json_Number( value ); }
Json_Value* Json_Value::create( double value ) { return new Json_Number( value ); }
Json_Value* Json_Value::create( std::string value ) { return new Json_String( value ); }
Json_Value* Json_Value::create( std::map<std::string, Json_Value*> value ) { return new Json_Object( value ); }
Json_Value* Json_Value::create( std::vector<Json_Value*> value ) { return new Json_Array( value ); }


Json_Null::Json_Null() {}

std::string Json_Null::to_string( int nesting )
{
    return "null";
}


Json_Bool::Json_Bool( bool value ) : data( value ) {}

std::string Json_Bool::to_string( int nesting )
{
    return ( data ? "true" : "false" );
}


Json_Number::Json_Number( long value ) : data( value ), value_type( LONG ) {}
Json_Number::Json_Number( double value ) : data( value ), value_type( DOUBLE ) {}

std::string Json_Number::to_string( int nesting )
{
    std::stringstream result;
    result << ( value_type == LONG ? static_cast<long>( data ) : data );
    return result.str();
}


Json_String::Json_String( std::string value )
{
    data = Json_Parser::insert_escapes( value );
}

std::string Json_String::to_string( int nesting )
{
    return "\"" + data + "\"";
}


Json_Object::Json_Object() {}
Json_Object::Json_Object( std::map<std::string, Json_Value*> value ) : data( value ) {}

Json_Object::Json_Object( std::string key1, Json_Value* val1 )
{
    data[key1] = val1;
}

Json_Object::Json_Object( std::string key1, Json_Value* val1, std::string key2, Json_Value* val2 )
{
    data[key1] = val1;
    data[key2] = val2;
}

Json_Object::~Json_Object()
{
    for ( std::map<std::string, Json_Value*>::iterator it = data.begin(); it != data.end(); ++it )
    { delete it->second; }
}

std::string Json_Object::to_string( int nesting )
{
    std::stringstream result;
    result << "{" << std::endl;

    for ( auto it = data.begin(); it != data.end(); ++it )
    {
        auto it_copy = it;
        result << std::string( ( nesting + 1 ) * indent_size, ' ' );
        result << "\"" << it->first << "\"" << " : ";
        result << it->second->to_string( nesting + 1 );
        result << ( ++it_copy == data.end() ? "" : "," );
        result << std::endl;
    }

    result << std::string( nesting * indent_size, ' ' ) << "}";
    return result.str();
}

void Json_Object::insert( std::string key, Json_Value* val )
{
    if ( data.find( key ) != data.end() )
    {
        delete data[key];
        data[key] = NULL;
    }

    data[key] = val;
}
void Json_Object::insert( std::string key )                  { insert( key, Json_Value::create() ); }
void Json_Object::insert( std::string key, bool val )        { insert( key, Json_Value::create( val ) ); }
void Json_Object::insert( std::string key, long val )        { insert( key, Json_Value::create( val ) ); }
void Json_Object::insert( std::string key, double val )      { insert( key, Json_Value::create( val ) ); }
void Json_Object::insert( std::string key, std::string val ) { insert( key, Json_Value::create( val ) ); }
void Json_Object::insert( std::string key, const char* val ) { insert( key, Json_Value::create( std::string( val ) ) ); }


Json_Array::Json_Array() {}
Json_Array::Json_Array( std::vector<Json_Value*> value ) : data( value ) {}

Json_Array::Json_Array( Json_Value* value[], int size )
{
    for ( int i = 0; i < size; i++ )
    { data.push_back( value[i] ); }
}

Json_Array::~Json_Array()
{
    for ( std::vector<Json_Value*>::iterator it = data.begin(); it != data.end(); ++it )
    { delete *it; }
}

std::string Json_Array::to_string( int nesting )
{
    std::stringstream result;
    result << "[";

    for ( auto it = data.begin(); it != data.end(); ++it )
    {
        result << ( *it )->to_string( nesting ) << ( ( it + 1 ) == data.end() ? "" : "," );
    }

    result << "]";
    return result.str();
}

void Json_Array::push_bask( Json_Value* val )
{
    data.push_back( val );
}

