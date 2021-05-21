//
// Created by Yaozy on 2021/5/11.
//

#ifndef BASE_TCPSERVER_H
#define BASE_TCPSERVER_H

#include <memory>
#include "ConnectManager.h"
#include "Acceptor.h"
#include "Epoller.h"
#include "../Thread/IndependentThreadPool.h"

namespace Base {
    namespace Net {
        namespace Tcp {

            class TcpServer {
            public:
                explicit TcpServer(const Sockets::InetAddress &localAddr);

                ~TcpServer();

                void Start();

                void Loop();

                void OnMessage(const std::shared_ptr<Connection> &connection, const std::shared_ptr<Buffer> &buffer);

            private:

                int fd_;
                Sockets::Epoll ep_;
                const Sockets::InetAddress localAddr_;
                std::shared_ptr<Base::IndependentThreadPool> independentThreadPool;

                std::shared_ptr<Base::IndependentThreadVoid> independentThreadVoid;
                std::shared_ptr<Acceptor> acceptor_;
                std::shared_ptr<ConnectManager> connectManager_;
            };
        }
    }
}


#endif //BASE_TCPSERVER_H
