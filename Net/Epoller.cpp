//
// Created by Yaozy on 2021/5/12.
//


#include <utility>
#include <functional>
#include "Epoller.h"
#include "Timer.h"
#include "Connection.h"

using namespace Base::Net::Tcp::Sockets;


Epoll::Epoll() : size_(0), loop(false) {
    fd_ = epoll_create(256);
    events_.resize(16);
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

int Epoll::AddTimer() {
    std::shared_ptr<Base::Thread::TEvent> timer_(new Base::Thread::TEvent);
    timer_->SetTime(10, 0, 10, 0);
    connections_[timer_->GetFd()] = timer_;
    AddEvent(timer_->GetFd());
    return 0;
}

int Epoll::AddListener(const int &fd, const std::shared_ptr<Base::Thread::Event> &lis) {
    AddEvent(fd);
    connections_[fd] = lis;
    return 0;
}

void Epoll::AddConnection(int fd, const Sockets::InetAddress &localAddr,
                          const Sockets::InetAddress &peerAddr) {
    if (events_.size() == size_)
        events_.resize(size_ * 2);

    std::shared_ptr<int> tie(new int(1));

    ties_[fd] = tie;
    std::shared_ptr<Connection> cn(new Connection(fd, localAddr, peerAddr, independentThreadVoid_));
    cn->SetOnMessage(std::bind(&Epoll::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    cn->SetDisConnect(
            std::bind(&Epoll::DELEvent, this, std::placeholders::_1));
    cn->SetUpdateFunc(
            std::bind(&Epoll::MODEvent, this, std::placeholders::_1, std::placeholders::_2));
    cn->SetTie(tie);
    cn->SetNoDelay(true);
    connections_[fd] = cn;
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
    std::shared_ptr<Connection> cn(std::dynamic_pointer_cast<Connection>(connections_[index]));
//    LOG_DEBUG("cn:"+std::to_string(cn.use_count())+"   "+std::to_string(connections_[index].use_count()));
    onMessage_(cn, buffer);
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
    int num = Wait(size_, events_, 100);
    for (int i = 0; i < num; ++i) {
        auto cn = connections_[events_[i].data.fd];
        cn->SetEvent(events_[i].events);
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

