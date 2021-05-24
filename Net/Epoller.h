//
// Created by Yaozy on 2021/5/12.
//

#ifndef BASE_EPOLLER_H
#define BASE_EPOLLER_H

#include <sys/epoll.h>
#include <vector>
#include <memory>
#include "../Thread/IndependentThread.h"
#include "CallBack.h"


namespace Base {
    namespace Net {
        namespace Tcp {
            class Connection;
            namespace Sockets {
                class InetAddress;
                class Epoll {
                public:
                    Epoll();

                    ~Epoll();

                    void AddConnection(int fd,const Sockets::InetAddress &localAddr,
                                 const Sockets::InetAddress &peerAddr);

                    int AddEvent(int fd);

                    void SetServerOnMessage(const OnMessage &func);

                    int DELEvent(const int &fd);

                    int MODEvent(int fd, int opt) const;

                    int Wait(int size, std::vector<struct epoll_event> &events ,const int &time) const;

                    void SetIndependentThreadVoid(const  std::shared_ptr<IndependentThreadVoid> &independentThreadVoid);

                    bool Looping() {return loop;}
                private:

                    void RemoveConnection(const int &fd);

                    void OnMessage(const int &, const std::shared_ptr<Buffer> &);

                    void WaitLoop();

                    std::map<int, std::shared_ptr<Connection>> connections_;
                    std::map<int, std::shared_ptr<int>> ties_;
                    std::shared_ptr<IndependentThreadVoid> independentThreadVoid_;
                    std::atomic<bool> loop;
                    int size_;
                    int fd_;
                    ::Base::Net::Tcp::OnMessage onMessage_;
                };
            }
        }
    }
}
#endif //BASE_EPOLLER_H
