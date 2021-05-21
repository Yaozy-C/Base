//
// Created by Yaozy on 2021/5/12.
//

#include "ConnectManager.h"
#include "Connection.h"
#include "../Public/Log.h"

using namespace Base::Net::Tcp;

ConnectManager::ConnectManager(const std::shared_ptr<IndependentThreadPool> &pool) : eventPool(pool), id_(0) {
    for (int i = 0; i < pool->GetSize(); ++i) {
        std::shared_ptr<Sockets::Epoll> epoll(new Sockets::Epoll);
        epolls.emplace_back(epoll);
    }
    independentThreadVoid = eventPool->GetIndependentThreadVoid(0);
    independentThreadVoid->AddTask(std::bind(&ConnectManager::EventLoop, this));
}

void
ConnectManager::NewConnection(int fd, const Sockets::InetAddress &peerAddr, const Sockets::InetAddress &localAddr) {

    auto ep = epolls[id_ % epolls.size()];
    auto loop = eventPool->GetIndependentThreadVoid(id_);

    std::shared_ptr<int> tie(new int(1));

    {
        std::unique_lock<std::mutex> lock(mtx_);
        ties_[fd] = tie;
        connections_[fd].reset(new Connection(fd, id_, localAddr, peerAddr, loop));
    }
    connections_[fd]->SetOnMessage(
            std::bind(&ConnectManager::ConnectOnMessage, this, std::placeholders::_1, std::placeholders::_2));
    connections_[fd]->SetDisConnect(
            std::bind(&ConnectManager::RemoveConnection, this, std::placeholders::_1, std::placeholders::_2));
    connections_[fd]->SetUpdateFunc(
            std::bind(&ConnectManager::ModConnection, this, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));
    connections_[fd]->SetTie(tie);

    loop->AddTask(std::bind(&Sockets::Epoll::AddEvent, ep, fd));
    id_++;
}


void ConnectManager::RemoveConnection(int fd, int index) {
    std::unique_lock<std::mutex> lock(mtx_);
    auto iter2 = ties_.find(fd);
    if (iter2 != ties_.end()) {
        ties_.erase(iter2);
        eventPool->GetIndependentThreadVoid(index)->AddTask(std::bind(&ConnectManager::RemoveInLoop, this, fd, index));
    }
}

void ConnectManager::RemoveInLoop(int fd, int index) {
    epolls[index % epolls.size()]->DELEvent(fd);
    std::unique_lock<std::mutex> lock(mtx_);
    auto iter = connections_.find(fd);
    if (iter != connections_.end()) {
        connections_.erase(iter);
    }
}

int ConnectManager::ModConnection(int fd, int index, int opt) {
    auto ep = epolls[index % epolls.size()];
    return ep->MODEvent(fd, opt);
}

void ConnectManager::SetServerOnMessage(const OnMessage &func) {
    onMessage_ = func;
}

void ConnectManager::ConnectOnMessage(const int &index, const std::shared_ptr<Buffer> &buffer) {
    std::unique_lock<std::mutex> lock(mtx_);
    auto iter = connections_.find(index);
    if (iter != connections_.end()) {
        onMessage_(connections_[index], buffer);
    }
}

void ConnectManager::WaitLoop(const std::shared_ptr<Sockets::Epoll> &epoll) {
    int size = epoll->GetSize();
    std::vector<struct epoll_event> events;
    events.resize(size);
    int num = epoll->Wait(size, events);
    for (int i = 0; i < num; ++i) {
        auto cn = connections_[events[i].data.fd];
        cn->SetEvent(events[i].events);
        cn->Loop();
    }
}

void ConnectManager::EventLoop() {

    for (int i = 0; i < epolls.size(); ++i) {
        eventPool->GetIndependentThreadVoid(i)->AddTask(std::bind(&ConnectManager::WaitLoop, this, epolls[i]));
    }
    independentThreadVoid->AddTask(std::bind(&ConnectManager::EventLoop, this));
}