//
// Created by Yaozy on 2021/5/11.
//

#include <sys/epoll.h>
#include <csignal>
#include "Connection.h"
#include "../Public/Log.h"

namespace {

    class IgnoreSigPipe {
    public:
        IgnoreSigPipe() {
            ::signal(SIGPIPE, SIG_IGN);
            LOG_DEBUG("Ignore SIGPIPE");
        }
    };

    IgnoreSigPipe ignoreSigPipe;
}

using namespace Base::Net::Tcp;


Connection::Connection(int sockfd, const Sockets::InetAddress &localAddr,
                       const Sockets::InetAddress &peerAddr,
                       const std::weak_ptr<IndependentThreadVoid> &independentThreadVoid) :
        socket_(new Sockets::Socket(sockfd)),
        peerAddr_(peerAddr), localAddr_(localAddr), input_(new Buffer),
        output_(new Buffer), events_(0) {

    independentThreadVoid_ = independentThreadVoid;
    socket_->SetKeepAlive(true);
}

Connection::~Connection() = default;

int Connection::Send(const std::string &message) {

    if (!output_->empty()) {
        output_->readFd(message.data(), message.length());
        int res = epollMod_(socket_->GetFd(),EPOLLOUT | EPOLLET);
        if (res < 0)
            ShutDown();
        return 0;
    }

    size_t left = message.size();
    size_t wd = 0;
    size_t size = SocketOpt::Write(socket_->GetFd(), message.data() + wd, left);
    left -= size;
    wd += size;

    if (left == 0) {
        int res = epollMod_(socket_->GetFd(), EPOLLIN | EPOLLET);
        if (res < 0)
            ShutDown();
        return 0;
    }
    if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        output_->readFd(message.data() + wd, left);
        int res = epollMod_(socket_->GetFd(), EPOLLOUT | EPOLLET);
        if (res < 0)
            ShutDown();
        return 0;
    }

    if (size <= 0)
        ShutDown();
    return 0;
}

int Connection::Read() {
    char buf[65535];
    std::string data;
    ssize_t size = 0;
    while ((size = SocketOpt::Read(socket_->GetFd(), buf, 65535)) > 0) {
        data.append(buf, size);
    }
    if (size < 0 && errno == EAGAIN) {
        input_->readFd(data.data(), data.length());
        if (onMessage_) {
            onMessage_(socket_->GetFd(), input_);
        }
        return 0;
    }
    if (size == 0) {
        return -1;
    }

    return 0;
}

void Connection::SetDisConnect(const DisConnect &func) {
    disConnect_ = func;
}

void Connection::SetOnMessage(const ConnOnMessage &func) {
    onMessage_ = func;
}

void Connection::SetTie(const std::shared_ptr<int> &tie) {
    tie_ = tie;
}

void Connection::SetUpdateFunc(const EpollMod &func) {
    epollMod_ = func;
}

void Connection::SetNoDelay(bool on) {
    socket_->SetTcpNoDelay(on);
}

std::string Connection::GetTcpInfo() {
    char buf[1024] = {0};
    socket_->GetTcpInfo(buf, sizeof buf);
    return std::string(buf);
}

std::string Connection::GetConnectionInfo() {
    return peerAddr_.ToIpPort();
}

void Connection::SetEvent(const uint32_t &event) {
    events_ = event;
}

int EventsToString(uint32_t ev) {
    if (ev & EPOLLIN)
        LOG_DEBUG("POLLIN");
    if (ev & EPOLLPRI)
        LOG_DEBUG("POLLPRI");
    if (ev & EPOLLOUT)
        LOG_DEBUG("POLLOUT");
    if (ev & EPOLLHUP)
        LOG_DEBUG("POLLHUP");
    if (ev & EPOLLRDHUP)
        LOG_DEBUG("POLLRDHUP");
    if (ev & EPOLLERR)
        LOG_DEBUG("POLLERR");
    if (ev & EINVAL)
        LOG_DEBUG("POLLNVAL");

    return 0;
}

int Connection::SendInLoop() {
    std::string message = output_->retrieveAllAsString();
    size_t left = message.size();
    size_t wd = 0;
    size_t size = 0;
    while ((size = SocketOpt::Write(socket_->GetFd(), message.data() + wd, left)) > 0) {
        left -= size;
        wd += size;
    }

    if (left == 0) {
        return 0;
    }
    if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        output_->readFd(message.data() + wd, left);
        return 1;
    }

    if (size <= 0) {
        return -1;
    }
    return 0;
}

void Connection::Loop() {
    std::shared_ptr<int> tie = tie_.lock();
    if (!tie) {
        return;
    }
    if ((events_ & EPOLLHUP) && !(events_ & EPOLLIN)) {
        ShutDown();
        return;
    }

    if (events_ & (EPOLLERR)) {
        ShutDown();
        return;
    }
    if (events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        int res = Read();
        if (res < 0)
            ShutDown();
        return;
    }
    if (events_ & EPOLLOUT) {
        int res = SendInLoop();
        if (res < 0) {
            ShutDown();
            return;
        }

        int events = EPOLLIN | EPOLLET;
        if (res > 0)
            events = EPOLLOUT | EPOLLET;

        res = epollMod_(socket_->GetFd(), events);
        if (res < 0)
            ShutDown();
    }
}

void Connection::ShutDownInLoop() {
    independentThreadVoid_.lock()->AddTask(std::bind(&Connection::ShutDown, this));
}

void Connection::ShutDown() {
    if (disConnect_) {
        disConnect_(socket_->GetFd());
    }
}

