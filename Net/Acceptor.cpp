//
// Created by Yaozy on 2021/5/11.
//

#include <fcntl.h>
#include <cassert>
#include <functional>
#include "Acceptor.h"

using namespace Base::Net::Tcp;

Acceptor::Acceptor(int sockfd, const Sockets::InetAddress &listenAddr) :
        acceptSocket(sockfd),
        nullFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(nullFd >= 0);
    acceptSocket.SetReuseAddr(true);
    acceptSocket.SetReusePort(true);
    acceptSocket.BindAddress(listenAddr);
    acceptSocket.SetTcpNoDelay(true);
}

Acceptor::~Acceptor() {
    ::close(nullFd);
}

void Acceptor::Listen() {
    acceptSocket.Listen();
}

void Acceptor::SetNewConnectCallBack(const std::function<void(int, const Sockets::InetAddress &)> &func) {
    func_ = func;
}

void Acceptor::SetEvent(const uint32_t &) {
}

void Acceptor::Loop() {
    Sockets::InetAddress peerAddr;
    int connfd = acceptSocket.Accept(&peerAddr);
    if (connfd >= 0) {
        std::string hostport = peerAddr.ToIpPort();
        if (func_) {
            func_(connfd, peerAddr);
        } else {
            SocketOpt::Close(connfd);
        }

    } else {
        if (errno == EMFILE) {
            ::close(nullFd);
            nullFd = ::accept(acceptSocket.GetFd(), nullptr, nullptr);
            ::close(nullFd);
            nullFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}