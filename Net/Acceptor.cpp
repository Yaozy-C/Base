//
// Created by Yaozy on 2021/5/11.
//

#include <fcntl.h>
#include <cassert>
#include <functional>
#include "Acceptor.h"
#include "../Public/Log.h"

using namespace Base::Net::Tcp;

Acceptor::Acceptor(int sockfd, const Sockets::InetAddress &listenAddr) :
        acceptSocket(sockfd),
        nullFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(nullFd >= 0);
    acceptSocket.SetReuseAddr(true);
    acceptSocket.SetReusePort(true);
    acceptSocket.BindAddress(listenAddr);
}

Acceptor::~Acceptor() {
    ::close(nullFd);
}

void Acceptor::Listen() {
//    listening = true;
    acceptSocket.Listen();
}

void Acceptor::SetNewConnectCallBack(const std::function<void(int, const Sockets::InetAddress &)> &func) {
    func_ = func;
}

void Acceptor::HandleRead() {
    Sockets::InetAddress peerAddr;
    int connfd = acceptSocket.Accept(&peerAddr);
    if (connfd >= 0) {
        std::string hostport = peerAddr.ToIpPort();
        LOG_DEBUG("Accepts of " + hostport);
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