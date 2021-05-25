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
        ep->AddTimer();
    }

    loop->AddTask(std::bind(&Sockets::Epoll::AddConnection, ep.get(), fd, localAddr, peerAddr));

    id_++;
}


void ConnectManager::SetServerOnMessage(const OnMessage &func) {
    onMessage_ = func;
}

void ConnectManager::SetListener(const int &fd, const std::shared_ptr<Event> &lis) {
    auto loop = loops_->GetIndependentThreadVoid(id_);
    if (!epolls[0]->Looping()) {
        epolls[0]->SetIndependentThreadVoid(loop);
        epolls[0]->SetServerOnMessage(onMessage_);
        epolls[0]->AddTimer();
        epolls[0]->AddListener(fd, lis);
    }
}