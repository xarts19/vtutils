#include "TcpSocket.h"

#define SIO_LOOPBACK_FAST_PATH  _WSAIOW(IOC_VENDOR,16)

TcpSocket::TcpSocket():	_socket(INVALID_SOCKET), _use_fastpath(false)
{
}

TcpSocket::~TcpSocket()
{
	close();
}

bool TcpSocket::bind(const char* address, const char* port, int flags)
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

	_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (_socket == INVALID_SOCKET)
	{
		printf("TcpSocket::bind - socket failed with error: %ld\n", WSAGetLastError());
	
		freeaddrinfo(result);
	
		return false;
	}

    if (_use_fastpath)
    {
        printf("TcpSocket::bind - Trying to set FAST_PATH...\n");
        set_fastpath();
    }

    status = ::bind( _socket, result->ai_addr, (int)result->ai_addrlen);
	if (status == SOCKET_ERROR)
	{
		printf("TcpSocket::bind - bind failed with error: %d\n", WSAGetLastError());
	
		freeaddrinfo(result);
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	
		return false;
	}

	freeaddrinfo(result);

	return true;
}

bool TcpSocket::listen()
{
    if (::listen(_socket, 1) != SOCKET_ERROR)
	{
		return true;
	}

	return false;
}

TcpSocket* TcpSocket::accept()
{
	TcpSocket* connection = new TcpSocket();

	int size = sizeof(struct sockaddr);
    connection->_socket = ::accept(_socket, (struct sockaddr *)&connection->_socket_address, &size);

	if (connection->_socket != INVALID_SOCKET)
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

bool TcpSocket::connect(const char* address, const char* port, int flags)
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
		_socket = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
		if (_socket == INVALID_SOCKET)
		{
			printf("TcpSocket::connect - socket failed with error: %ld\n", WSAGetLastError());
		
			return false;
		}

        if (_use_fastpath)
        {
            printf("TcpSocket::connect - Trying to set FAST_PATH...\n");
            set_fastpath();
        }

		// Connect to server.
        status = ::connect(_socket, i->ai_addr, (int)i->ai_addrlen);
		if (status == SOCKET_ERROR)
		{
			printf("TcpSocket::connect - connect failed with error: %ld\n", WSAGetLastError());

			closesocket(_socket);
			_socket = INVALID_SOCKET;
			continue;
		}
	
		break;
	}

	freeaddrinfo(result);

	if (_socket == INVALID_SOCKET)
	{
		return false;
	}

	return true;
}

int TcpSocket::receive(char *buffer, int length)
{
    while ( true )
    {
        int received = ::recv(_socket, buffer, length, 0);
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

int TcpSocket::send(const char *buffer, int length)
{
    while (true)
    {
        int sent = ::send(_socket, buffer, length, 0);

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

bool TcpSocket::close()
{
	int status = SOCKET_ERROR;

	if (_socket != INVALID_SOCKET)
	{
		status = closesocket(_socket);
		_socket = INVALID_SOCKET;
	}

	return status != SOCKET_ERROR;
}

int TcpSocket::set_fastpath()
{
    unsigned long bytes_ret = -1;
    int tmp1;
    int tmp2;
    if ( SOCKET_ERROR == WSAIoctl(_socket, SIO_LOOPBACK_FAST_PATH, &tmp1, sizeof(tmp1), &tmp2, sizeof(tmp2), &bytes_ret, 0, 0))
    {
        if (WSAGetLastError() != WSAEOPNOTSUPP)
        {
            printf("TcpSocket::set_fastpath - WSAIoctl error: %d\n", WSAGetLastError());
            return 1;
        }
        else
            printf("TcpSocket::set_fastpath - SIO_LOOPBACK_FAST_PATH not supported\n");
    }

    return 0;
}