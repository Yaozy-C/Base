//
// Created by Yaozy on 2021/5/11.
//

#include <functional>
#include "TcpServer.h"
#include "Connection.h"
#include "Log.h"

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
    work_.reset(new EventLoop);

    connectManager_.reset(new ConnectManager(independentThreadPool));
    connectManager_->SetServerOnMessage(
            [this](auto && PH1, auto && PH2) { OnMessage(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    acceptor_.reset(new Acceptor(fd_, localAddr_));
    acceptor_->Listen();
    NewConnectionCallback func = [capture0 = connectManager_.get(), this](auto && PH1, auto && PH2) { capture0->NewConnection(std::forward<decltype(PH1)>(PH1), localAddr_, std::forward<decltype(PH2)>(PH2)); };
    acceptor_->SetNewConnectCallBack(func);

    connectManager_->SetListener(fd_, acceptor_);
    DEBUG << "start listen:" << localAddr_.ToIpPort();
}

void TcpServer::OnMessage(const std::shared_ptr<Connection> &connection, const std::shared_ptr<Buffer> &buffer) {

    work_->AddTask([this, connection, buffer] { OnMessageInWorker(connection, buffer); });
//    std::string data = buffer->retrieveAllAsString();
//    connection->Send(data);
}

void TcpServer::OnMessageInWorker(const std::shared_ptr<Connection> &connection,
                                  const std::shared_ptr<Buffer> &buffer) {
    std::string data = buffer->retrieveAllAsString();
//    LOG_DEBUG(data);
    connection->Send(data);
}