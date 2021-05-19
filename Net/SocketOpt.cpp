//
// Created by Yaozy on 2021/4/29.
//

#include "SocketOpt.h"
#include "../Public/Log.h"
#include <cassert>

using namespace Base::Net::Tcp;


void SocketOpt::MemZero(void *p, size_t n) {
    memset(p, 0, n);
}

const struct sockaddr *SocketOpt::SockAddrCast(const struct sockaddr_in6 *addr) {
    return static_cast<const struct sockaddr *>(ConversionTypeBToA<const void *>(addr));
}

struct sockaddr *SocketOpt::SockAddrCast(struct sockaddr_in6 *addr) {
    return static_cast<struct sockaddr *>(ConversionTypeBToA<void *>(addr));
}

const struct sockaddr *SocketOpt::SockAddrCast(const struct sockaddr_in *addr) {
    return static_cast<const struct sockaddr *>(ConversionTypeBToA<const void *>(addr));
}

const struct sockaddr_in *SocketOpt::SockAddrInCast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in *>(ConversionTypeBToA<const void *>(addr));
}

const struct sockaddr_in6 *SocketOpt::SockAddrIn6Cast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in6 *>(ConversionTypeBToA<const void *>(addr));
}

const struct sockaddr_in *SocketOpt::SockAddrCast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in *>(ConversionTypeBToA<const void *>(addr));
}

int SocketOpt::CreateNoBlock(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_ERROR("create no block err");
    }
    return sockfd;
}

void SocketOpt::Bind(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG_ERROR(std::to_string(ret));
    }
}

void SocketOpt::Listen(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_ERROR(std::to_string(ret));
    }
}

int SocketOpt::Accept(int sockfd, struct sockaddr_in6 *addr) {
    auto addrLen = static_cast<socklen_t>(sizeof *addr);

    int connfd = ::accept4(sockfd, SocketOpt::SockAddrCast(addr), &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        int savedErrno = errno;
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_ERROR(std::to_string(savedErrno));
                break;
            default:
                LOG_ERROR(std::to_string(savedErrno));
                break;
        }
    }
    return connfd;
}

int SocketOpt::Connect(int sockfd, const struct sockaddr *addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t SocketOpt::Read(int sockfd, void *buf, size_t len) {
    return ::read(sockfd, buf, len);
}

ssize_t SocketOpt::Readv(int sockfd, const struct iovec *iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t SocketOpt::Write(int sockfd, const void *buf, size_t len) {
    return ::write(sockfd, buf, len);
}

void SocketOpt::Close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_ERROR("close fd err");
    }
}

void SocketOpt::ShutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        perror("SocketOpt::shutdownWrite:");
        LOG_ERROR("SocketOpt::shutdownWrite");
    }
}

void SocketOpt::ToIpPort(char *buf, size_t size,
                         const struct sockaddr *addr) {
    ToIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in *addr4 = SockAddrCast(addr);
    uint16_t port = be16toh(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

void SocketOpt::ToIp(char *buf, size_t size, const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in *addr4 = SockAddrCast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        const struct sockaddr_in6 *addr6 = SockAddrIn6Cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void SocketOpt::FromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = htobe16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
//        LOG_SYSERR << "SocketOpt::fromIpPort";
        LOG_ERROR("inet_pton err");
    }
}

void SocketOpt::FromIpPort(const char *ip, uint16_t port, struct sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = htobe16(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
//        LOG_SYSERR << "SocketOpt::fromIpPort";
        LOG_ERROR("inet_pton err");
    }
}

int SocketOpt::GetSocketError(int sockfd) {
    int optval;
    auto optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in6 SocketOpt::GetLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr{};
    MemZero(&localaddr, sizeof localaddr);
    auto addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, SockAddrCast(&localaddr), &addrlen) < 0) {
        LOG_ERROR("SocketOpt::GetLocalAddr");
    }
    return localaddr;
}

struct sockaddr_in6 SocketOpt::GetPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr{};
    MemZero(&peeraddr, sizeof peeraddr);
    auto addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, SockAddrCast(&peeraddr), &addrlen) < 0) {
//        LOG_SYSERR << "SocketOpt::getLocalAddr";
        LOG_ERROR("getpeername err");
    }
    return peeraddr;
}

bool SocketOpt::IsSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = GetLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = GetPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in *laddr4 = reinterpret_cast<struct sockaddr_in *>(&localaddr);
        const struct sockaddr_in *raddr4 = reinterpret_cast<struct sockaddr_in *>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
               && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port
               && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    } else {
        return false;
    }
}
