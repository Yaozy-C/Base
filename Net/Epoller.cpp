//
// Created by Yaozy on 2021/5/12.
//


#include <utility>
#include "Epoller.h"

using namespace Base::Net::Tcp::Sockets;


Epoll::Epoll() : size_(0) {
    fd_ = epoll_create(256);
}

Epoll::~Epoll() = default;

int Epoll::AddEvent(int fd) {
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    int res = epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev);
    if (res >= 0)
        size_++;
    return res;
}

int Epoll::DELEvent(int fd) {
    struct epoll_event ev{};
    ev.data.fd = fd;

    int res = epoll_ctl(fd_, EPOLL_CTL_DEL, fd, &ev);
    if (res >= 0)
        size_--;
    return res;
}

int Epoll::MODEvent(int fd, int opt) const {
    struct epoll_event ev{};
    ev.events = opt;
    ev.data.fd = fd;
    return epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev);
}

int Epoll::Wait(int size, std::vector<struct epoll_event> &events) const {
    return epoll_wait(fd_, &*events.begin(), size, 1);
}

int Epoll::GetSize() {
    return size_;
}
