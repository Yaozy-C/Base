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

Connection::~Connection() =default ;

void Connection::Send(const std::string &message) {

    if (message.length() >= 0) {

        size_t size = SocketOpt::Write(socket_->GetFd(), message.data(), message.size());
        if (size >= 0) {


//            LOG_DEBUG("Send:" + message);
        } else {
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET ) {
                    LOG_DEBUG("Send: EPIPE :" + message);

                }
            }
        }
    }
}


void Connection::Read() {
    char extrabuf[65536];

    while (true) {
        const ssize_t n = SocketOpt::Read(socket_->GetFd(), extrabuf, 65536);
        if (n <= 0) {
            return;
        } else {

            std::string a(extrabuf);
//            LOG_DEBUG("recv: " + std::to_string(a.length()) + " " + a);
        }
    }

    //TODO

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
    if (!tie){
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
        Read();

        int res = epollMod_(socket_->GetFd(), index_, POLLOUT);
        if (res < 0)
            ShutDown();
        return;
    }
    if (events_ & POLLOUT) {
        Send("123213213213");
        int res = epollMod_(socket_->GetFd(), index_, POLLIN);
        if (res < 0)
            ShutDown();
    }
}

void Connection::Loop() {
    std::shared_ptr<int> tie = tie_.lock();
    if (!tie){
        return;
    }
    independentThreadVoid_.lock()->AddTask(std::bind(&Connection::LoopInThread, this));
}

void Connection::ShutDown() {

    if (disConnect_) {
        disConnect_(socket_->GetFd(), index_);
    }
}

