#include "VTBaseSocket.h"

#include "VTNet.h"

#include <winsock2.h>
#include <Ws2tcpip.h>


struct VT::BaseSocket::Impl
{
    SocketType type;
};


VT::BaseSocket::BaseSocket(SocketType socketType)
    : pimpl_(new Impl)
{
    // make sure WSA is initialized
    static VT::Net& n = VT::Net::instance();

    pimpl_->type = socketType;
}


VT::BaseSocket::~BaseSocket()
{
    delete pimpl_;
}




const int BaseSocket::BS_SOCKET_ERROR     = SOCKET_ERROR;
const int BaseSocket::BS_SOMAXCONN         = SOMAXCONN;
const int BaseSocket::BS_EWOULDBLOCK     = WSAEWOULDBLOCK;
const int BaseSocket::BS_ETIMEDOUT         = WSAETIMEDOUT;
const int BaseSocket::BS_EHOSTUNREACH     = WSAEHOSTUNREACH;
const int BaseSocket::BS_AF_INET6         = AF_INET6;
const int BaseSocket::BS_AF_INET         = AF_INET;
const int BaseSocket::BS_SOCK_STREAM     = SOCK_STREAM;
const int BaseSocket::BS_IPPROTO_TCP     = IPPROTO_TCP;
const int BaseSocket::BS_AI_PASSIVE     = AI_PASSIVE;
const int BaseSocket::BS_NI_MAXHOST        = NI_MAXHOST;
const int BaseSocket::BS_NI_MAXSERV        = NI_MAXSERV;
const int BaseSocket::BS_NI_NUMERICSERV    = NI_NUMERICSERV;

class AddrInfoWin : public AddrInfo
{
public:
    AddrInfoWin(struct addrinfo* info): _addrinfo(info), _addrinfo_current(info) {}
    ~AddrInfoWin() { freeaddrinfo(_addrinfo); }

    int GetFamily()             {return _addrinfo->ai_family;}
    int GetSockType()             {return _addrinfo->ai_socktype;}
    int GetProtocol()             {return _addrinfo->ai_protocol;}
    struct sockaddr* GetAddr()     {return _addrinfo->ai_addr;}
    size_t GetAddrLen()         {return _addrinfo->ai_addrlen;}

    bool HasCurrent()
    {
        return _addrinfo_current != NULL;
    }
    void Next()
    {
        _addrinfo_current = _addrinfo_current->ai_next;
    }

private:
    struct addrinfo*    _addrinfo;
    struct addrinfo*    _addrinfo_current;

};

AddrInfo* AddrInfo::create(const char* address, const char* port, struct hints* hint, int* status)
{
    struct addrinfo *result = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = hint->ai_family;
    hints.ai_socktype = hint->ai_socktype;
    hints.ai_protocol = hint->ai_protocol;
    hints.ai_flags = hint->ai_flags;
    *status = getaddrinfo(address, port, &hints, &result);
    return new AddrInfoWin(result);
}



class BaseSocketWin: public BaseSocket
{

public:

    BaseSocketWin(SOCKET sock): _socket(sock) {}
    BaseSocketWin(SOCKET sock, sockaddr_in addr): _socket(sock), _socketAddress(addr) {}
    ~BaseSocketWin() {}

    int Close()
    {
        shutdown(_socket, 2);    //needed to wake the thread waiting in accept call
        int status = closesocket(_socket);
        _socket = INVALID_SOCKET;
        return status;
    }

    BaseSocket* Accept()
    {
        struct sockaddr_in addr;
        int size = sizeof(struct sockaddr);
        SOCKET new_socket = accept(_socket, (struct sockaddr *)&addr, &size);
        return new BaseSocketWin(new_socket, addr);
    }

    int Bind(AddrInfo* addr)
    {
        AddrInfoWin* a = static_cast<AddrInfoWin*>(addr);
        return bind(_socket, a->GetAddr(), (int)a->GetAddrLen());
    }

    int Listen(int backlog)
    {
        return listen(_socket, backlog);
    }

    int Connect(AddrInfo* addr)
    {
        AddrInfoWin* a = static_cast<AddrInfoWin*>(addr);
        return connect(_socket, a->GetAddr(), (int)a->GetAddrLen());
    }

    int Recieve(char *buffer, int length, int /*flags*/)
    {
        return recv(_socket, buffer, length, 0);
    }

    int Send(const char *buffer, int length, int /*flags*/)
    {
        return send(_socket, buffer, length, 0);
    }

    int MyGetNameInfo(char* host, size_t hostlen, char* serv, size_t servlen, int flags)
    {
        return getnameinfo((struct sockaddr *)&_socketAddress, sizeof(sockaddr), host, (DWORD)hostlen, serv, (DWORD)servlen, flags);
    }

    void SetNonBlocking()
    {
        unsigned long nonBloking = TRUE;
        ioctlsocket(_socket, FIONBIO, &nonBloking);
    }

    bool isInvalid()
    {
        return _socket == INVALID_SOCKET;
    }

private:
    SOCKET         _socket;
    sockaddr_in    _socketAddress;

};

BaseSocket* BaseSocket::create()
{
    return new BaseSocketWin(INVALID_SOCKET);
}

BaseSocket* BaseSocket::create(AddrInfo* addr)
{
    AddrInfoWin* a = static_cast<AddrInfoWin*>(addr);
    SOCKET sock = socket(a->GetFamily(), a->GetSockType(), a->GetProtocol());
    return new BaseSocketWin(sock);
}

int BaseSocket::InitSockets()
{
    WORD versionWanted = MAKEWORD(2, 2);
    WSADATA wsaData;
    if(WSAStartup(versionWanted, &wsaData) != 0 )
    {
        return -1;
    }
    return 0;
}

int BaseSocket::CleanupSockets()
{
    // terminates use of the Winsock
    return WSACleanup();
}

int BaseSocket::GetLastError()
{
    return WSAGetLastError();
}
