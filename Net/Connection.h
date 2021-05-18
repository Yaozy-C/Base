//
// Created by Yaozy on 2021/5/11.
//

#ifndef BASE_CONNECTION_H
#define BASE_CONNECTION_H

#include <memory>
#include <functional>
#include <atomic>
#include "InetAddress.h"
#include "Socket.h"
#include "../Public/Buffer.h"
#include "../Thread/IndependentThread.h"


namespace Base {
    namespace Net {
        namespace Tcp {

            typedef std::function<void(int, int)> DisConnect;

            typedef std::function<int(int, int, int)> EpollMod;

            class Connection {
            public:
                explicit Connection(int sockfd, const int &index, const Sockets::InetAddress &localAddr,
                                    const Sockets::InetAddress &peerAddr,
                                    const std::weak_ptr<IndependentThreadVoid> &independentThreadVoid);

                ~Connection();

                int Send(const std::string &message);

                void ShutDown();

                int Read();

                void SetTie(const std::shared_ptr<int> &tie);

                void SetDisConnect(const DisConnect &func);

                void SetUpdateFunc(const EpollMod &func);

                std::string GetTcpInfo();

                std::string GetConnectionInfo();

                void SetEvent(const uint32_t &event);

                void Loop();


            private:

                void LoopInThread();

                const int index_;
                std::unique_ptr<Sockets::Socket> socket_;
                std::weak_ptr<int> tie_;
                const Sockets::InetAddress peerAddr_;
                const Sockets::InetAddress localAddr_;
                Buffer input_;
                Buffer output_;
                EpollMod epollMod_;
                DisConnect disConnect_;
                std::atomic<uint32_t> events_;
                std::weak_ptr<IndependentThreadVoid> independentThreadVoid_;
            };
        }
    }
}


#endif //BASE_CONNECTION_H
