#include "VTTcpSocket.h"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

namespace VT
{
    class TcpSocketImpl
    {
    public:
        TcpSocketImpl() : _socket(INVALID_SOCKET), _socket_address() {}

        SOCKET      _socket;
        sockaddr_in _socket_address;
    };
}

VT::TcpSocket::TcpSocket():	pimpl_(new TcpSocketImpl())
{ }

VT::TcpSocket::~TcpSocket()
{
    close();
    delete pimpl_;
}

bool VT::TcpSocket::bind(const char* address, const char* port, int flags)
{
    // Check if socket already created and close it
    close();

    // Create new socket and bind it
    struct addrinfo *result = NULL;
    struct addrinfo hints;
    int status;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(address, port, &hints, &result);
    if (status != 0)
    {
        printf("TcpSocket::bind - getaddrinfo failed with error: %d\n", status);
    
        return false;
    }

    pimpl_->_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (pimpl_->_socket == INVALID_SOCKET)
    {
        printf("TcpSocket::bind - socket failed with error: %ld\n", WSAGetLastError());
    
        freeaddrinfo(result);
    
        return false;
    }

    status = ::bind( pimpl_->_socket, result->ai_addr, (int)result->ai_addrlen);
    if (status == SOCKET_ERROR)
    {
        printf("TcpSocket::bind - bind failed with error: %d\n", WSAGetLastError());
    
        freeaddrinfo(result);
        closesocket(pimpl_->_socket);
        pimpl_->_socket = INVALID_SOCKET;
    
        return false;
    }

    freeaddrinfo(result);

    return true;
}

bool VT::TcpSocket::listen()
{
    if (::listen(pimpl_->_socket, 1) != SOCKET_ERROR)
    {
        return true;
    }

    return false;
}

VT::TcpSocket* VT::TcpSocket::accept()
{
    TcpSocket* connection = new TcpSocket();

    int size = sizeof(struct sockaddr);
    connection->pimpl_->_socket = ::accept(pimpl_->_socket, (struct sockaddr *)&connection->pimpl_->_socket_address, &size);

    if (connection->pimpl_->_socket != INVALID_SOCKET)
    {
        return connection;
    }
    else
    {
        printf("TcpSocket::Accept - accept failed with error: %ld\n", WSAGetLastError());

        delete connection;
        return 0;
    }
}

bool VT::TcpSocket::connect(const char* address, const char* port, int flags)
{
    // Check if socket already created and close it
    close();

    struct addrinfo *result = NULL;
    struct addrinfo	hints;

    int status;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    status = getaddrinfo(address, port, &hints, &result);
    if (status != 0)
    {
        printf("TcpSocket::connect - getaddrinfo failed with error: %d\n", status);
    
        return false;
    }

    // Attempt to connect to an address until one succeeds
    for (struct addrinfo *i = result; i != NULL; i = i->ai_next)
    {
        // Create a SOCKET for connecting to server
        pimpl_->_socket = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
        if (pimpl_->_socket == INVALID_SOCKET)
        {
            printf("TcpSocket::connect - socket failed with error: %ld\n", WSAGetLastError());
        
            return false;
        }

        // Connect to server.
        status = ::connect(pimpl_->_socket, i->ai_addr, (int)i->ai_addrlen);
        if (status == SOCKET_ERROR)
        {
            printf("TcpSocket::connect - connect failed with error: %ld\n", WSAGetLastError());

            closesocket(pimpl_->_socket);
            pimpl_->_socket = INVALID_SOCKET;
            continue;
        }
    
        break;
    }

    freeaddrinfo(result);

    if (pimpl_->_socket == INVALID_SOCKET)
    {
        return false;
    }

    return true;
}

int VT::TcpSocket::receive(char *buffer, int length)
{
    while ( true )
    {
        int received = ::recv(pimpl_->_socket, buffer, length, 0);
        if (received == length)
        {
            return 1;
        }
        else if (received > 0 && received < length)
        {
            buffer += received;
            length -= received;
        }
        else if (received == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
}

int VT::TcpSocket::send(const char *buffer, int length)
{
    while (true)
    {
        int sent = ::send(pimpl_->_socket, buffer, length, 0);

        if (sent == length)
        {
            return 1;
        }
        else if (sent > 0 && sent < length)
        {
            buffer += sent;
            length -= sent;
        }
        else
        {
            return -1;
        }
    }
}

bool VT::TcpSocket::close()
{
    int status = SOCKET_ERROR;

    if (pimpl_->_socket != INVALID_SOCKET)
    {
        status = closesocket(pimpl_->_socket);
        pimpl_->_socket = INVALID_SOCKET;
    }

    return status != SOCKET_ERROR;
}
