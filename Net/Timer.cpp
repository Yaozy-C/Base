//
// Created by Yaozy on 2021/5/24.
//

#include "Timer.h"
#include "../Public/Log.h"
#include "../Thread/Pool.h"
#include "SocketOpt.h"
#include <sys/timerfd.h>

using namespace Base::Net::Tcp;


Timer::Timer() : fd_(-1), event_(0) {
}

Timer::~Timer() {
    SocketOpt::Close(fd_);
}

void Timer::Init() {
    fd_ = ::timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd_ < 0) {
        abort();
    }
}

int Timer::GetFd() const {
    return fd_;
}

void Timer::SetTime() const {
    struct itimerspec new_value{};

    new_value.it_value.tv_sec = 2;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_nsec = 0;
    int ret = timerfd_settime(fd_, 0, &new_value, nullptr);
    if (ret < 0) {
        ::close(fd_);
    }
}

void Timer::Loop() {
    uint64_t exp = 0;
    size_t res = read(fd_, &exp, sizeof(uint64_t));
    if (res < 0)
        LOG_ERROR("Timer::Loop()");
}

void Timer::SetEvent(const uint32_t &event) {
    event_ = event;
}