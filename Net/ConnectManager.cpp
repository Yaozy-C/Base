//
// Created by Yaozy on 2021/5/12.
//

#include "ConnectManager.h"
#include "../Public/Log.h"

using namespace Base::Net::Tcp;

ConnectManager::ConnectManager(const std::shared_ptr<IndependentThreadPool> &pool) : id_(0), eventPool(pool) {

    for (int i = 0; i < pool->GetSize(); ++i) {
        std::shared_ptr<Sockets::Epoll> epoll(new Sockets::Epoll);
        epolls.emplace_back(epoll);
    }

    eventPool->GetIndependentTimeLoop()->AddTaskAt(std::bind(&ConnectManager::EventLoop, this));

}

void
ConnectManager::NewConnection(int fd, const Sockets::InetAddress &peerAddr, const Sockets::InetAddress &localAddr) {

    auto ep = epolls[id_ % epolls.size()];
    auto loop = eventPool->GetIndependentThreadVoid(id_);

    std::shared_ptr<int> tie (new int(1));

    std::shared_ptr<Connection> ptr(new Connection(fd, id_, localAddr, peerAddr, loop));
    ptr->SetDisConnect(
            std::bind(&ConnectManager::RemoveConnection, this, std::placeholders::_1, std::placeholders::_2));
    ptr->SetUpdateFunc(std::bind(&ConnectManager::ModConnection, this, std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3));
    ptr->SetTie(tie);
    loop->AddTask(std::bind(&Sockets::Epoll::AddEvent, ep, fd));
    {
        std::unique_lock<std::mutex> lock(mtx_);
        ties_[fd] = tie;
        connections_[fd] = ptr;
    }
    id_++;
}


void ConnectManager::RemoveConnection(int fd, int index) {
    epolls[index % epolls.size()]->DELEvent(fd);
    auto iter2 = ties_.find(fd);
    if (iter2 != ties_.end()) {
        std::unique_lock<std::mutex> lock(mtx_);
        ties_.erase(iter2);
        eventPool->GetIndependentThreadVoid(index)->AddTask(std::bind(&ConnectManager::RemoveInLoop, this, fd, index));
    }

}

void ConnectManager::RemoveInLoop(int fd, int index) {
    epolls[index % epolls.size()]->DELEvent(fd);
    auto iter = connections_.find(fd);
    if (iter!=connections_.end()) {
        std::unique_lock<std::mutex> lock(mtx_);
        connections_.erase(iter);
    }

    if (connections_.empty()) {
        LOG_DEBUG("empty");
    }
}

int ConnectManager::ModConnection(int fd, int index, int opt) {
    auto ep = epolls[index % epolls.size()];
    return ep->MODEvent(fd, opt);
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
//        eventPool->GetIndependentTimeLoop()->AddTaskAt(std::bind(&ConnectManager::WaitLoop, this, epolls[i]));
    }
    LOG_DEBUG(std::to_string(connections_.size()));
    eventPool->GetIndependentTimeLoop()->AddTaskAt(std::bind(&ConnectManager::EventLoop, this));
}