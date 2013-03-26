#pragma once

#include "VTUtil.h"

#include <string>

namespace VT
{
    class BaseSocket
    {
    public:
        enum SocketType
        {
            TcpSocket,
            UdpSocket,
            UnknownSocketType = -1
        };

        enum NetworkLayerProtocol
        {
            IPv4Protocol,
            IPv6Protocol,
            UnknownNetworkLayerProtocol = -1
        };

        enum SocketError
        {
            ConnectionRefusedError,
            RemoteHostClosedError,
            HostNotFoundError,
            SocketAccessError,
            SocketResourceError,
            SocketTimeoutError,
            DatagramTooLargeError,
            NetworkError,
            AddressInUseError,
            SocketAddressNotAvailableError,
            UnsupportedSocketOperationError,
            UnfinishedSocketOperationError,

            UnknownSocketError = -1
        };

        enum SocketState
        {
            UnconnectedState,
            HostLookupState,
            ConnectingState,
            ConnectedState,
            BoundState,
            ListeningState,
            ClosingState
        };

        /*
        enum SocketOption
        {
            LowDelayOption, // TCP_NODELAY
            KeepAliveOption, // SO_KEEPALIVE
            MulticastTtlOption, // IP_MULTICAST_TTL
            MulticastLoopbackOption // IP_MULTICAST_LOOPBACK
        };
        */

        enum OpenMode
        {
            Read,
            Write,
            ReadWrite
        };

        explicit BaseSocket(SocketType socketType = TcpSocket);
        virtual ~BaseSocket();

        void connect_to_host(const std::string& hostName, int port, OpenMode mode = ReadWrite);
        void disconnect_from_host();

        bool is_valid() const;

        long long bytes_available() const;
        long long bytes_to_write() const;

        bool can_read_line() const;

        int local_port() const;
        std::string local_address() const;
        int peer_port() const;
        std::string peer_address() const;
        std::string peer_name() const;

        long long read_buffer_size() const;
        void set_read_buffer_size(long long size);

        void abort();

        int socket_descriptor() const;
        bool set_socket_descriptor(int socketDescriptor, SocketState state = ConnectedState,
            OpenMode openMode = ReadWrite);

        /*
        void setSocketOption(SocketOption option, const Variant &value);
        Variant socketOption(SocketOption option);
        */

        SocketType socket_type() const;
        SocketState state() const;
        SocketError error() const;

        // for synchronous access
        bool wait_for_connected(int msecs = 30000);
        bool wait_for_ready_read(int msecs = 30000);
        bool wait_for_bytes_written(int msecs = 30000);
        bool wait_for_disconnected(int msecs = 30000);

    protected:
        long long read_data(char *data, long long maxlen);
        long long read_line_data(char *data, long long maxlen);
        long long write_data(const char *data, long long len);

        void set_socket_state(SocketState state);
        void set_socket_error(SocketError socketError);
        void set_local_port(long long port);
        void set_local_address(const std::string& address);
        void set_peer_port(long long port);
        void set_peer_address(const std::string& address);
        void set_peer_name(const std::string& name);

    private:
        VT_DISABLE_COPY(BaseSocket);

        struct Impl;
        Impl* pimpl_;
    };
}