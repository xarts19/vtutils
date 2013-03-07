#pragma once

#include <stdlib.h>

class AddrInfo
{
public:
    struct hints
    {
        int ai_flags;
        int ai_family;
        int ai_socktype;
        int ai_protocol;
    };

    static AddrInfo* create(const char* address, const char* port, struct hints* hint, int* status);
    virtual ~AddrInfo() {}

    virtual bool HasCurrent() = 0;
    virtual void Next() = 0;

};


class BaseSocket
{

public:
    static const int BS_SOCKET_ERROR;
    static const int BS_SOMAXCONN;
    static const int BS_EWOULDBLOCK;
    static const int BS_ETIMEDOUT;
    static const int BS_EHOSTUNREACH;
    static const int BS_AF_INET6;
    static const int BS_AF_INET;
    static const int BS_SOCK_STREAM;
    static const int BS_IPPROTO_TCP;
    static const int BS_AI_PASSIVE;
    static const int BS_NI_MAXHOST;
    static const int BS_NI_MAXSERV;
    static const int BS_NI_NUMERICSERV;

    static BaseSocket* create();
    static BaseSocket* create(AddrInfo* addr);
    virtual ~BaseSocket() {}

    static int InitSockets();
    static int CleanupSockets();
    static int GetLastError();

    virtual int         Close() = 0;
    virtual BaseSocket* Accept() = 0;
    virtual int         Bind(AddrInfo* addr) = 0;
    virtual int         Listen(int backlog) = 0;
    virtual int         Connect(AddrInfo* addr) = 0;
    virtual int         Recieve(char *buffer, int length, int flags) = 0;
    virtual int         Send(const char *buffer, int length, int flags) = 0;
    // stupid macros
    virtual int         MyGetNameInfo(char* host, size_t hostlen, char* serv, size_t servlen, int flags) = 0;
    virtual void        SetNonBlocking() = 0;
    virtual bool        isInvalid() = 0;

};
