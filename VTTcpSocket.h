#pragma once

namespace VT
{
    class TcpSocketImpl;

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

    private:
        TcpSocket(const TcpSocket&);
        TcpSocket& operator=(const TcpSocket& rhs);

    private:
        TcpSocketImpl* pimpl_;
    };
}