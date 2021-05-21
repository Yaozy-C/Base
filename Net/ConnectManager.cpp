//
// Created by Yaozy on 2021/5/12.
//

#include <sys/eventfd.h>
#include "ConnectManager.h"
#include "Connection.h"
#include "../Public/Log.h"

using namespace Base::Net::Tcp;

ConnectManager::ConnectManager(const std::shared_ptr<IndependentThreadPool> &pool) : loops_(pool), id_(0) {
    for (int i = 0; i < pool->GetSize(); ++i) {
        std::shared_ptr<Sockets::Epoll> epoll(new Sockets::Epoll);
        epolls.emplace_back(epoll);

//        int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
//        if (fd < 0)
//            abort();
//        efds_.emplace_back(fd);
//        epoll->AddEvent(fd);
    }


    independentThreadVoid = loops_->GetIndependentThreadVoid(0);
}

void
ConnectManager::NewConnection(int fd, const Sockets::InetAddress &localAddr, const Sockets::InetAddress &peerAddr) {

    int index = id_ % epolls.size();
    auto ep = epolls[index];
    auto loop = loops_->GetIndependentThreadVoid(id_);
    if (!ep->Looping()){
        ep->SetIndependentThreadVoid(loop);
        loop->AddTask(std::bind(&ConnectManager::WaitLoop, this, ep));
    }

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
//    Wake(efds_[index]);
    id_++;
}


void ConnectManager::RemoveConnection(int fd, int index) {
    std::unique_lock<std::mutex> lock(mtx_);
    auto iter2 = ties_.find(fd);
    if (iter2 != ties_.end()) {
        ties_.erase(iter2);
        loops_->GetIndependentThreadVoid(index)->AddTask(std::bind(&ConnectManager::RemoveInLoop, this, fd, index));
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
    int newIndex = index % epolls.size();
    auto ep = epolls[newIndex];
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

//void ConnectManager::Wake(int fd) {
//    uint64_t one = 1;
//    ssize_t n = SocketOpt::Write(fd, &one, sizeof one);
//}
//
//void ConnectManager::Read(int fd) {
//    uint64_t one = 1;
//    ssize_t n = SocketOpt::Read(fd, &one, sizeof one);
//}

void ConnectManager::WaitLoop(const std::shared_ptr<Sockets::Epoll> &epoll) {
    int size = epoll->GetSize();
    std::vector<struct epoll_event> events;
    events.resize(size);
    int num = epoll->Wait(size, events, 0);
    for (int i = 0; i < num; ++i) {
        auto cn = connections_[events[i].data.fd];
        cn->SetEvent(events[i].events);
        cn->Loop();
    }
    epoll->GetLoop()->AddTask(std::bind(&ConnectManager::WaitLoop, this, epoll));
}