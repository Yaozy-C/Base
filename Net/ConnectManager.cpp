//
// Created by Yaozy on 2021/5/12.
//

#include <sys/eventfd.h>
#include "ConnectManager.h"
#include "Connection.h"

using namespace Base::Net::Tcp;

ConnectManager::ConnectManager(const std::shared_ptr<IndependentThreadPool> &pool) : loops_(pool), id_(0) {
    for (int i = 0; i < pool->GetSize(); ++i) {
        std::shared_ptr<Sockets::Epoll> epoll(new Sockets::Epoll);
        epolls.emplace_back(epoll);
    }
}

void
ConnectManager::NewConnection(int fd, const Sockets::InetAddress &localAddr, const Sockets::InetAddress &peerAddr) {

    int index = id_ % epolls.size();
    auto ep = epolls[index];
    auto loop = loops_->GetIndependentThreadVoid(id_);
    if (!ep->Looping()) {
        ep->SetIndependentThreadVoid(loop);
        ep->SetServerOnMessage(onMessage_);
    }

    loop->AddTask(std::bind(&Sockets::Epoll::AddConnection,ep.get(),fd,localAddr,peerAddr));

    id_++;
}


void ConnectManager::SetServerOnMessage(const OnMessage &func) {
    onMessage_ = func;
}