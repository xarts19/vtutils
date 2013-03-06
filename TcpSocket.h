#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>

class TcpSocket
{
public:

	TcpSocket();
	~TcpSocket();

	bool connect(const char* address, const char* port, int flags = 0);
	bool bind(const char* address, const char* port, int flags = 0);

	bool listen();
	TcpSocket* accept();
	int receive(char *buffer, int length);
	int send(const char *buffer, int length);

	bool close();

    void use_fastpath() { _use_fastpath = true; };
    int set_fastpath();

private:
	SOCKET		_socket;
	sockaddr_in	_socket_address;
    bool        _use_fastpath;
};
