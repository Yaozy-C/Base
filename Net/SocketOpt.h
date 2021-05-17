//
// Created by Yaozy on 2021/4/29.
//

#ifndef BASE_SOCKETOPT_H
#define BASE_SOCKETOPT_H

#include <arpa/inet.h>
#include <cerrno>
#include <unistd.h>
#include <sys/uio.h>
#include <cstring>
#include <cstdio>


template<typename A, typename B>
inline A ConversionTypeBToA(B const &f) {
    return f;
}

namespace Base {
    namespace Net {
        namespace Tcp {
            namespace SocketOpt {
                int CreateNoBlock(sa_family_t family);

                int Connect(int sockfd, const struct sockaddr *addr);

                void Bind(int sockfd, const struct sockaddr *addr);

                void Listen(int sockfd);

                int Accept(int sockfd, struct sockaddr_in6 *addr);

                ssize_t Read(int sockfd, void *buf, size_t len);

                ssize_t Readv(int sockfd, const struct iovec *iov, int iovcnt);

                ssize_t Write(int sockfd, const void *buf, size_t len);

                void Close(int sockfd);

                void ShutdownWrite(int sockfd);

                void ToIpPort(char *buf, size_t size, const struct sockaddr *addr);

                void ToIp(char *buf, size_t size, const struct sockaddr *addr);

                void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

                void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in6 *addr);

                int GetSocketError(int sockfd);

                void MemZero(void *p, size_t n);

                const struct sockaddr *SockAddrCast(const struct sockaddr_in6 *addr);

                struct sockaddr *SockAddrCast(struct sockaddr_in6 *addr);

                const struct sockaddr *SockAddrCast(const struct sockaddr_in *addr);

                const struct sockaddr_in *SockAddrInCast(const struct sockaddr *addr);

                const struct sockaddr_in6 *SockAddrIn6Cast(const struct sockaddr *addr);

                const sockaddr_in *SockAddrCast(const sockaddr *addr);

                struct sockaddr_in6 GetLocalAddr(int sockfd);

                struct sockaddr_in6 GetPeerAddr(int sockfd);

                bool IsSelfConnect(int sockfd);

            }
        }
    }
}

#endif //BASE_SOCKETOPT_H
