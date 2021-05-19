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

namespace Base {
    namespace Net {
        namespace Tcp {

            class ConnectManager {
            public:
                explicit ConnectManager(const std::shared_ptr<IndependentThreadPool> &pool);

                void NewConnection(int fd, const Sockets::InetAddress &peerAddr, const Sockets::InetAddress &localAddr);

                void RemoveConnection(int fd, int index);

                void EventLoop();

                int ModConnection(int fd, int index, int opt);

                void SetServerOnMessage(const OnMessage &func);

                void ConnectOnMessage(const int &index, const std::shared_ptr<Buffer>&);

            private:

                void RemoveInLoop(int fd, int index);

                void WaitLoop(const std::shared_ptr<Sockets::Epoll> &epoll);
                std::mutex mtx_;
                std::shared_ptr<IndependentThreadPool> eventPool;
                std::map<int, std::shared_ptr<Connection>> connections_;
                std::map<int,std::shared_ptr<int>> ties_;
                std::atomic<int> id_;
                std::vector<std::shared_ptr<Sockets::Epoll>> epolls;
                OnMessage onMessage_;
            };
        }
    }
}

#endif //BASE_CONNECTMANAGER_H
