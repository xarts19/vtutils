#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "BaseSocket.h"

#define INVALID_SOCKET -1

const int BaseSocket::BS_SOCKET_ERROR 	= -1;
const int BaseSocket::BS_SOMAXCONN 		= SOMAXCONN;
const int BaseSocket::BS_EWOULDBLOCK 	= EWOULDBLOCK;
const int BaseSocket::BS_ETIMEDOUT 		= ETIMEDOUT;
const int BaseSocket::BS_EHOSTUNREACH 	= EHOSTUNREACH;
const int BaseSocket::BS_AF_INET6 		= AF_INET6;
const int BaseSocket::BS_AF_INET 		= AF_INET;
const int BaseSocket::BS_SOCK_STREAM 	= SOCK_STREAM;
const int BaseSocket::BS_IPPROTO_TCP 	= IPPROTO_TCP;
const int BaseSocket::BS_AI_PASSIVE 	= AI_PASSIVE;
const int BaseSocket::BS_NI_MAXHOST		= NI_MAXHOST;
const int BaseSocket::BS_NI_MAXSERV		= NI_MAXSERV;
const int BaseSocket::BS_NI_NUMERICSERV	= NI_NUMERICSERV;

class AddrInfoLinux : public AddrInfo
{
public:
	AddrInfoLinux(struct addrinfo* info): _addrinfo(info), _addrinfo_current(info) {}
	~AddrInfoLinux() { freeaddrinfo(_addrinfo); }

	int GetFamily() 			{return _addrinfo->ai_family;}
	int GetSockType() 			{return _addrinfo->ai_socktype;}
	int GetProtocol() 			{return _addrinfo->ai_protocol;}
	struct sockaddr* GetAddr() 	{return _addrinfo->ai_addr;}
	size_t GetAddrLen() 		{return _addrinfo->ai_addrlen;}

	bool HasCurrent() { return _addrinfo_current != NULL; }
	void Next() { _addrinfo_current = _addrinfo_current->ai_next; }

private:
	struct addrinfo*	_addrinfo;
	struct addrinfo*	_addrinfo_current;

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
	return new AddrInfoLinux(result);
}



class BaseSocketLinux: public BaseSocket
{

public:

	BaseSocketLinux(int sock): _socket(sock) {}
	BaseSocketLinux(int sock, sockaddr_in addr): _socket(sock), _socketAddress(addr) {}
	~BaseSocketLinux() {}

	int Close()
	{
		shutdown(_socket, 2);	//needed to wake the thread waiting in accept call
		int status = close(_socket);
		_socket = INVALID_SOCKET;
		return status;
	}

	BaseSocket* Accept()
	{
		struct sockaddr_in addr;
		unsigned int size = sizeof(struct sockaddr);
		int new_socket = accept(_socket, (struct sockaddr *)&addr, &size);
		return new BaseSocketLinux(new_socket, addr);
	}

	int Bind(AddrInfo* addr)
	{
		AddrInfoLinux* a = static_cast<AddrInfoLinux*>(addr);
		return bind(_socket, a->GetAddr(), a->GetAddrLen());
	}

	int Listen(int backlog)
	{
		return listen(_socket, backlog);
	}

	int Connect(AddrInfo* addr)
	{
		AddrInfoLinux* a = static_cast<AddrInfoLinux*>(addr);
		return connect(_socket, a->GetAddr(), a->GetAddrLen());
	}

	int Recieve(char *buffer, int length, int /*flags*/)
	{
		return recv(_socket, buffer, length, 0);
	}

	int Send(const char *buffer, int length, int /*flags*/)
	{
		return send(_socket, buffer, length, 0);
	}

	int GetNameInfo(char* host, size_t hostlen, char* serv, size_t servlen, int flags)
	{
		return getnameinfo((struct sockaddr *)&_socketAddress, sizeof(sockaddr), host, hostlen, serv, servlen, flags);
	}

	void SetNonBlocking()
	{
		int socket_flags;
		socket_flags = fcntl(_socket, F_GETFL, 0);
		fcntl(_socket, F_SETFL, socket_flags | O_NONBLOCK);
	}

	bool isInvalid()
	{
		return _socket == INVALID_SOCKET;
	}

private:
	int 		_socket;
	sockaddr_in	_socketAddress;

};

BaseSocket* BaseSocket::create()
{
	return new BaseSocketLinux(INVALID_SOCKET);
}

BaseSocket* BaseSocket::create(AddrInfo* addr)
{
	AddrInfoLinux* a = static_cast<AddrInfoLinux*>(addr);
	int sock = socket(a->GetFamily(), a->GetSockType(), a->GetProtocol());
	return new BaseSocketLinux(sock);
}

int BaseSocket::InitSockets()
{
	signal(SIGPIPE, SIG_IGN);
	return 0;
}

int BaseSocket::CleanupSockets()
{
	return 0;
}

int BaseSocket::GetLastError()
{
	return errno;
}
