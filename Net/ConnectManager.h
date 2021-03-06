//
// Created by Yaozy on 2021/5/12.
//

#ifndef BASE_CONNECTMANAGER_H
#define BASE_CONNECTMANAGER_H

#include <map>
#include <atomic>
#include <vector>
#include "InetAddress.h"
#include "CallBack.h"
#include "Epoller.h"
#include "../Thread/IndependentThreadPool.h"

namespace Base::Net::Tcp {

    class ConnectManager {
    public:
        explicit ConnectManager(const std::shared_ptr<IndependentThreadPool> &pool);

        void NewConnection(int fd, const Sockets::InetAddress &localAddr, const Sockets::InetAddress &peerAddr);

        void SetServerOnMessage(const OnMessage &func);

        void SetListener(const int &fd,const std::shared_ptr<Base::Thread::Event> &lis);

    private:

        std::shared_ptr<IndependentThreadPool> loops_;

        std::atomic<int> id_;
        std::vector<std::shared_ptr<Sockets::Epoll>> epolls;
        OnMessage onMessage_;
    };
}

#endif //BASE_CONNECTMANAGER_H
