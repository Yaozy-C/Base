//
// Created by Yaozy on 2021/5/11.
//

#include <poll.h>
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


Connection::Connection(int sockfd, const int &index, const Sockets::InetAddress &localAddr,
                       const Sockets::InetAddress &peerAddr,
                       const std::weak_ptr<IndependentThreadVoid> &independentThreadVoid) :
        socket_(new Sockets::Socket(sockfd)),
        index_(index), peerAddr_(peerAddr), localAddr_(localAddr), events_(0) {

    input_.SetHead(std::string("2"));
    output_.SetHead(std::string("2"));
    independentThreadVoid_ = independentThreadVoid;
    socket_->SetKeepAlive(true);
}

Connection::~Connection() = default;

int Connection::Send(const std::string &message) {

    size_t left = message.size();
    size_t wd = 0;
    size_t size = 0;
    while ((size = SocketOpt::Write(socket_->GetFd(), message.data() + wd, left)) > 0) {
        left -= size;
        wd += size;
    }

    if (left == 0) {
        return 1;
    }
    if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 1;
    }

    if (size <= 0) {
        return -1;
    }

    return 0;
}


int Connection::Read() {
    char buf[4096];
    std::string data;
    ssize_t size = 0;
    while ((size = SocketOpt::Read(socket_->GetFd(), buf, 4096)) > 0) {
        data.append(buf, size);
    }
    if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        //todo
        return 0;
    }
    if (size < 0) {
        LOG_DEBUG("read err :" + std::to_string(SocketOpt::GetSocketError(socket_->GetFd())));
    }
    return -1;
}

void Connection::SetDisConnect(const DisConnect &func) {
    disConnect_ = func;
}

void Connection::SetTie(const std::shared_ptr<int> &tie) {
    tie_ = tie;
}

void Connection::SetUpdateFunc(const EpollMod &func) {
    epollMod_ = func;
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
    if (ev & POLLIN)
        LOG_DEBUG("POLLIN");
    if (ev & POLLPRI)
        LOG_DEBUG("POLLPRI");
    if (ev & POLLOUT)
        LOG_DEBUG("POLLOUT");
    if (ev & POLLHUP)
        LOG_DEBUG("POLLHUP");
    if (ev & POLLRDHUP)
        LOG_DEBUG("POLLRDHUP");
    if (ev & POLLERR)
        LOG_DEBUG("POLLERR");
    if (ev & POLLNVAL)
        LOG_DEBUG("POLLNVAL");

    return 0;
}


void Connection::LoopInThread() {

    //fixme bug

    std::shared_ptr<int> tie = tie_.lock();
    if (!tie) {
        return;
    }

    if ((events_ & POLLHUP) && !(events_ & POLLIN)) {
        ShutDown();
        return;
    }

    if (events_ & POLLNVAL) {
        return;
    }

    if (events_ & (POLLERR | POLLNVAL)) {
        ShutDown();
        return;
    }
    if (events_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        int res = Read();
        if (res < 0) {
            ShutDown();
            return;
        }

        res = epollMod_(socket_->GetFd(), index_, POLLOUT);
        if (res < 0)
            ShutDown();
        return;
    }
    if (events_ & POLLOUT) {
        int res = Send("1234567890");

        if (res < 0) {
            ShutDown();
            return;
        }

        int events = POLLIN;
        if (res > 0) {
            events = POLLOUT;
        }

        res = epollMod_(socket_->GetFd(), index_, events);
        if (res < 0)
            ShutDown();
    }
}

void Connection::Loop() {
    std::shared_ptr<int> tie = tie_.lock();
    if (!tie) {
        return;
    }
    independentThreadVoid_.lock()->AddTask(std::bind(&Connection::LoopInThread, this));
}

void Connection::ShutDown() {

    if (disConnect_) {
        disConnect_(socket_->GetFd(), index_);
    }
}

