//
// Created by Yaozy on 2021/5/11.
//

#include <functional>
#include "TcpServer.h"

using namespace Base::Net::Tcp;

TcpServer::TcpServer(const Sockets::InetAddress &localAddr) : localAddr_(localAddr),
                                                              independentThreadPool(new IndependentThreadPool(3, 40)) {

    fd_ = SocketOpt::CreateNoBlock(localAddr_.Family());

}

TcpServer::~TcpServer() {
    ::close(fd_);
}

void TcpServer::Start() {

    connectManager_.reset(new ConnectManager(independentThreadPool));
    acceptor_.reset(new Acceptor(fd_, localAddr_));
    acceptor_->Listen();
    NewConnectionCallback func = std::bind(&TcpServer::NewConnection, this, std::placeholders::_1,
                                           std::placeholders::_2);
    acceptor_->SetNewConnectCallBack(func);
    ep_.AddEvent(fd_);
    independentThreadPool->GetIndependentTimeLoop()->AddTaskAt(std::bind(&TcpServer::Loop, this));
}

void TcpServer::Loop() {
    std::vector<struct epoll_event> events;
    events.resize(1);
    int num = ep_.Wait(1, events);
    if (num > 0) {
        acceptor_->HandleRead();
        ep_.MODEvent(fd_, EPOLLIN);
    }
    independentThreadPool->GetIndependentTimeLoop()->AddTaskAt(std::bind(&TcpServer::Loop, this));
}

void TcpServer::NewConnection(int fd, const Sockets::InetAddress &peerAddr) {
    connectManager_->NewConnection(fd, peerAddr, localAddr_);
}