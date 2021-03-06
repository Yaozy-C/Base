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

namespace Base::Net::Tcp {

    class TcpServer {
    public:
        explicit TcpServer(const Sockets::InetAddress &localAddr);

        ~TcpServer();

        void Start();

        void OnMessage(const std::shared_ptr<Connection> &connection, const std::shared_ptr<Buffer> &buffer);

        void OnMessageInWorker(const std::shared_ptr<Connection> &connection, const std::shared_ptr<Buffer> &buffer);

    private:

        int fd_;
        const Sockets::InetAddress localAddr_;
        std::shared_ptr<Base::IndependentThreadPool> independentThreadPool;

        std::shared_ptr<Base::EventLoop> independentThreadVoid;
        std::shared_ptr<Base::EventLoop> work_;
        std::shared_ptr<Acceptor> acceptor_;
        std::shared_ptr<ConnectManager> connectManager_;
    };
}


#endif //BASE_TCPSERVER_H
