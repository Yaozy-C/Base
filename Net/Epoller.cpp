//
// Created by Yaozy on 2021/5/12.
//


#include <utility>
#include <functional>
#include "Epoller.h"
#include "Connection.h"

using namespace Base::Net::Tcp::Sockets;


Epoll::Epoll() : size_(0), loop(false) {
    fd_ = epoll_create(256);
    events.resize(16);
}

Epoll::~Epoll() {
    ::close(fd_);
}

int Epoll::AddEvent(int fd) {
    struct epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    int res = epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
    if (res >= 0)
        size_++;
    return res;
}

void Epoll::AddConnection(int fd, const Sockets::InetAddress &localAddr,
                          const Sockets::InetAddress &peerAddr) {
    if (events.size() == size_)
        events.resize(size_*2);

    std::shared_ptr<int> tie(new int(1));

    ties_[fd] = tie;
    connections_[fd].reset(new Connection(fd, localAddr, peerAddr, independentThreadVoid_));

    connections_[fd]->SetOnMessage(std::bind(&Epoll::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    connections_[fd]->SetDisConnect(
            std::bind(&Epoll::DELEvent, this, std::placeholders::_1));
    connections_[fd]->SetUpdateFunc(
            std::bind(&Epoll::MODEvent, this, std::placeholders::_1, std::placeholders::_2));
    connections_[fd]->SetTie(tie);
    AddEvent(fd);
}

int Epoll::DELEvent(const int &fd) {
    ties_.erase(fd);
    independentThreadVoid_->AddTask(std::bind(&Epoll::RemoveConnection, this, fd));
    return 0;
}

void Epoll::RemoveConnection(const int &fd) {
    connections_.erase(fd);
    struct epoll_event ev{};
    ev.data.fd = fd;

    int res = epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &ev);
    if (res >= 0)
        size_--;
}

void Epoll::OnMessage(const int &index, const std::shared_ptr<Buffer> &buffer) {
    onMessage_(connections_[index], buffer);
}

int Epoll::MODEvent(int fd, int opt) const {
    struct epoll_event ev{};
    ev.events = opt;
    ev.data.fd = fd;
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Epoll::Wait(int size, std::vector<struct epoll_event> &events, const int &time) const {
    return epoll_wait(fd_, &*events.begin(), size, time);
}

void Epoll::WaitLoop() {
    int num = Wait(size_, events, 0);
    for (int i = 0; i < num; ++i) {
        auto cn = connections_[events[i].data.fd];
        cn->SetEvent(events[i].events);
        cn->Loop();
    }
    independentThreadVoid_->AddTask(std::bind(&Epoll::WaitLoop, this));
}

void Epoll::SetIndependentThreadVoid(const std::shared_ptr<IndependentThreadVoid> &independentThreadVoid) {
    independentThreadVoid_ = independentThreadVoid;
    loop = true;
    independentThreadVoid_->AddTask(std::bind(&Epoll::WaitLoop, this));
}

void Epoll::SetServerOnMessage(const ::Base::Net::Tcp::OnMessage &func) {
    onMessage_ = func;
}

