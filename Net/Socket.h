//
// Created by Yaozy on 2021/5/11.
//

#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include "InetAddress.h"

namespace Base::Net::Tcp::Sockets {

    class Socket {
    public:
        explicit Socket(int sockfd) : sockfd_(sockfd) {}

        ~Socket();

        [[nodiscard]] int GetFd() const { return sockfd_; }

        bool GetTcpInfo(struct tcp_info *) const;

        bool GetTcpInfo(char *buf, int len) const;

        void BindAddress(const InetAddress &address) const;

        void Listen() const;

        int Accept(InetAddress *peeraddr) const;

        void ShutDownWrite() const;

        void SetReusePort(bool on) const;

        void SetTcpNoDelay(bool on) const;

        void SetReuseAddr(bool on) const;

        void SetKeepAlive(bool on) const;

    private:
        const int sockfd_;
    };
}

#endif //BASE_SOCKET_H
