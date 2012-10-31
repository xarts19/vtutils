#pragma once

#include <string>
#include <map>
#include <vector>

class Json_Value
{
public:
    virtual ~Json_Value() {}

    static Json_Value* create();
    static Json_Value* create( bool value );
    static Json_Value* create( long value );
    static Json_Value* create( double value );
    static Json_Value* create( std::string value );
    static Json_Value* create( std::map<std::string, Json_Value*> value );
    static Json_Value* create( std::vector<Json_Value*> value );

    virtual std::string to_string( int nesting = 0 ) = 0;

    static const int indent_size = 4;
};


class Json_Null : public Json_Value
{
public:
    Json_Null();
    std::string to_string( int nesting = 0 );
};

class Json_Bool : public Json_Value
{
public:
    Json_Bool( bool value );
    std::string to_string( int nesting = 0 );
private:
    bool data;
};

class Json_Number : public Json_Value
{
public:
    Json_Number( long value );
    Json_Number( double value );
    std::string to_string( int nesting = 0 );
private:
    double data;
    enum {LONG, DOUBLE} value_type;
};

class Json_String : public Json_Value
{
public:
    Json_String( std::string value );
    std::string to_string( int nesting = 0 );
private:
    std::string data;
};

class Json_Object : public Json_Value
{
public:
    Json_Object();
    Json_Object( std::map<std::string, Json_Value*> value );
    Json_Object::Json_Object( std::string key1, Json_Value* val1 );
    Json_Object::Json_Object( std::string key1, Json_Value* val1, std::string key2, Json_Value* val2 );
    ~Json_Object();

    std::string to_string( int nesting = 0 );
    void insert( std::string key, Json_Value* val );
    void insert( std::string key );
    void insert( std::string key, bool val );
    void insert( std::string key, long val );
    void insert( std::string key, double val );
    void insert( std::string key, std::string val );
    void insert( std::string key, const char* val );
private:
    std::map<std::string, Json_Value*> data;
};

class Json_Array : public Json_Value
{
public:
    Json_Array();
    Json_Array( std::vector<Json_Value*> value );
    Json_Array( Json_Value* value[], int size );
    ~Json_Array();

    std::string to_string( int nesting = 0 );
    void push_bask( Json_Value* val );
private:
    std::vector<Json_Value*> data;
};



namespace Json_Parser
{
    std::string insert_escapes( const std::string& str );
    std::string bytes_to_string( unsigned long long bytes );
};

