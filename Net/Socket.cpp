//
// Created by Yaozy on 2021/5/11.
//

#include <netinet/tcp.h>
#include <netinet/in.h>
#include "Socket.h"
#include "SocketOpt.h"
#include "../Public/Log.h"

using namespace Base::Net::Tcp::Sockets;

Socket::~Socket() {
    SocketOpt::Close(sockfd_);
}

bool Socket::GetTcpInfo(struct tcp_info *tcpInfo) const {
    socklen_t len = sizeof(*tcpInfo);
    Net::Tcp::SocketOpt::MemZero(tcpInfo, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpInfo, &len) == 0;
}

bool Socket::GetTcpInfo(char *buf, int len) const {
    struct tcp_info tcpInfo{};

    if (GetTcpInfo(&tcpInfo)) {
        snprintf(buf, len, "unrecovered=%u "
                           "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u "
                           "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpInfo.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                 tcpInfo.tcpi_rto,          // Retransmit timeout in usec
                 tcpInfo.tcpi_ato,          // Predicted tick of soft clock in usec
                 tcpInfo.tcpi_snd_mss,
                 tcpInfo.tcpi_rcv_mss,
                 tcpInfo.tcpi_lost,         // Lost packets
                 tcpInfo.tcpi_retrans,      // Retransmitted packets out
                 tcpInfo.tcpi_rtt,          // Smoothed round trip time in usec
                 tcpInfo.tcpi_rttvar,       // Medium deviation
                 tcpInfo.tcpi_snd_ssthresh,
                 tcpInfo.tcpi_snd_cwnd,
                 tcpInfo.tcpi_total_retrans);
        return true;
    }
    return false;
}

void Socket::BindAddress(const InetAddress &address) const {
    SocketOpt::Bind(sockfd_, address.GetSockAddr());
}

void Socket::Listen() const {
    SocketOpt::Listen(sockfd_);
}

int Socket::Accept(InetAddress *peeraddr) const {
    struct sockaddr_in6 addr{};
    SocketOpt::MemZero(&addr, sizeof addr);
    int connfd = SocketOpt::Accept(sockfd_, &addr);
    if (connfd >= 0)
        peeraddr->SetSocketAddrInet6(addr);
    return connfd;
}

void Socket::ShutDownWrite() const {
    SocketOpt::ShutdownWrite(sockfd_);
}

void Socket::SetReusePort(bool on) const {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_ERROR("SetReusePort failed");
    }
}

void Socket::SetTcpNoDelay(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::SetReuseAddr(bool on) const {
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_ERROR("SetReuseAddr failed");
    }
}

void Socket::SetKeepAlive(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}
