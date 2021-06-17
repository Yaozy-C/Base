//
// Created by Yaozy on 2021/5/11.
//

#include <functional>
#include "TcpServer.h"
#include "Connection.h"

using namespace Base::Net::Tcp;

TcpServer::TcpServer(const Sockets::InetAddress &localAddr) : localAddr_(localAddr),
                                                              independentThreadPool(new IndependentThreadPool(2)) {

    fd_ = SocketOpt::CreateNoBlock(localAddr_.Family());
    independentThreadVoid = independentThreadPool->GetIndependentThreadVoid(0);
}

TcpServer::~TcpServer() {
    ::close(fd_);
}

void TcpServer::Start() {
    work_.reset(new IndependentThreadVoid);

    connectManager_.reset(new ConnectManager(independentThreadPool));
    connectManager_->SetServerOnMessage(
            std::bind(&TcpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    acceptor_.reset(new Acceptor(fd_, localAddr_));
    acceptor_->Listen();
    NewConnectionCallback func = std::bind(&ConnectManager::NewConnection, connectManager_.get(), std::placeholders::_1,
                                           localAddr_, std::placeholders::_2);
    acceptor_->SetNewConnectCallBack(func);

    connectManager_->SetListener(fd_, acceptor_);
}

void TcpServer::OnMessage(const std::shared_ptr<Connection> &connection, const std::shared_ptr<Buffer> &buffer) {

    work_->AddTask(std::bind(&TcpServer::OnMessageInWorker, this,connection,buffer));
//    std::string data = buffer->retrieveAllAsString();
//    connection->Send(data);
}

void TcpServer::OnMessageInWorker(const std::shared_ptr<Connection> &connection,
                                  const std::shared_ptr<Buffer> &buffer) {
    std::string data = buffer->retrieveAllAsString();
    connection->Send(data);
}