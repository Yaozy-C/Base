//
// Created by Yaozy on 2021/5/24.
//

#include "Timer.h"
#include "Log.h"
#include "SocketOpt.h"
#include <sys/timerfd.h>

#include <utility>

using namespace Base::Thread;


TEvent::TEvent() : _fd(-1), _event(0), _interval(false) {
    _fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (_fd < 0) {
        abort();
    }
}

TEvent::~TEvent() {
    Net::Tcp::SocketOpt::Close(_fd);
}

int TEvent::GetFd() const {
    return _fd;
}

void
TEvent::SetTime(const int &second, const int &nSecond, const int &intervalSecond, const int &intervalNSecond) {
    struct itimerspec new_value{};

    new_value.it_value.tv_sec = second;
    new_value.it_value.tv_nsec = nSecond;
    new_value.it_interval.tv_sec = intervalSecond;
    new_value.it_interval.tv_nsec = intervalNSecond;
    if (intervalSecond || intervalNSecond)
        _interval = true;

    int ret = timerfd_settime(_fd, 0, &new_value, nullptr);
    if (ret < 0) {
        ::close(_fd);
    }
}

void TEvent::Loop() {
    uint64_t exp = 0;
    size_t res = read(_fd, &exp, sizeof(uint64_t));
    if (res < 0)
        LOG_ERROR("Timer::Loop()");

    if (_func)
        _func();

    if (!_disconnect)
        LOG_ERROR("_disconnect is null");
    if (!_interval)
        _disconnect(_fd);
}

void TEvent::SetEvent(const uint32_t &event) {
    _event = event;
}

void TEvent::SetCallBack(std::function<void()> func) {
    _func = std::move(func);
}

void TEvent::RegisterRemove(std::function<void(const int &)> disconnect) {
    _disconnect = std::move(disconnect);
}

Timer::Timer() : _fd(-1), size_(0) {
    _fd = epoll_create(256);
    events_.resize(16);
}

Timer::~Timer() {
    ::close(_fd);
}

int Timer::AddEvent(int fd, const std::shared_ptr<TEvent> &event) {
    struct epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    int res = epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &ev);
    if (res >= 0)
        size_++;

    std::unique_lock<std::mutex> _lock;
    auto iter = connections_.find(fd);
    if (iter != connections_.end())
        LOG_ERROR("event is existence");
    else
        connections_[event->GetFd()] = event;

    return res;
}

int Timer::DELEvent(const int &fd) {
    struct epoll_event ev{};
    ev.data.fd = fd;

    int res = epoll_ctl(_fd, EPOLL_CTL_DEL, fd, &ev);
    if (res >= 0)
        size_--;
    return res;
}

int Timer::GetSize() {
    return size_;
}

bool Timer::Empty() {
    return size_ <= 1;
}

void Timer::RemoveEvent(const int &fd) {
    std::unique_lock<std::mutex> _lock;

    auto iter = connections_.find(fd);
    if (iter != connections_.end()) {
        int res = DELEvent(fd);
        if (res < 0)
            LOG_ERROR("remove fd error");
        connections_.erase(fd);
    }
}

int Timer::Wait(int size, std::vector<struct epoll_event> &events, const int &time) const {
    return epoll_wait(_fd, &*events.begin(), size, time);
}

void Timer::WaitLoop() {
    int num = Wait(size_, events_, 100);
    for (int i = 0; i < num; ++i) {
        auto cn = connections_[events_[i].data.fd];
        cn->SetEvent(events_[i].events);
        cn->Loop();
    }
}