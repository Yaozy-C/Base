//
// Created by Yaozy on 2021/5/11.
//

#ifndef BASE_ACCEPTOR_H
#define BASE_ACCEPTOR_H

#include <functional>
#include "Socket.h"
#include "Event.h"

namespace Base::Net::Tcp {

    typedef std::function<void(int, const Base::Net::Tcp::Sockets::InetAddress &)> NewConnectionCallback;

    class Acceptor : public Base::Thread::Event {
    public:
        explicit Acceptor(int sockfd, const Sockets::InetAddress &listenAddr);

        ~Acceptor();

        void Listen();

        void Loop() override;

        void SetEvent(const uint32_t &) override;

        void SetNewConnectCallBack(const NewConnectionCallback &func);

    private:
        Sockets::Socket acceptSocket;
        int nullFd;

        NewConnectionCallback func_;
    };
}


#endif //BASE_ACCEPTOR_H
