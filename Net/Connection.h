//
// Created by Yaozy on 2021/5/11.
//

#ifndef BASE_CONNECTION_H
#define BASE_CONNECTION_H

#include <memory>
#include <functional>
#include <atomic>
#include "InetAddress.h"
#include "Event.h"
#include "Socket.h"
#include "CallBack.h"
#include "../Public/Buffer.h"
#include "../Thread/IndependentThread.h"


namespace Base::Net::Tcp {

    class Connection : public Event {
    public:
        explicit Connection(int sockfd, const Sockets::InetAddress &localAddr,
                            const Sockets::InetAddress &peerAddr,
                            const std::weak_ptr<IndependentThreadVoid> &independentThreadVoid);

        ~Connection();

        int Send(const std::string &message);

        void ShutDown();

        int Read();

        void SetTie(const std::shared_ptr<int> &tie);

        void SetDisConnect(const DisConnect &func);

        void SetOnMessage(const ConnOnMessage &func);

        void SetUpdateFunc(const EpollMod &func);

        std::string GetTcpInfo();

        std::string GetConnectionInfo();

        void SetEvent(const uint32_t &event) override;

        void Loop() override;

        void ShutDownInLoop();

    private:

        int SendInLoop();

        std::unique_ptr<Sockets::Socket> socket_;
        std::weak_ptr<int> tie_;
        const Sockets::InetAddress peerAddr_;
        const Sockets::InetAddress localAddr_;
        std::shared_ptr<Buffer> input_;
        std::shared_ptr<Buffer> output_;
        EpollMod epollMod_;
        DisConnect disConnect_;
        std::atomic<uint32_t> events_;
        std::weak_ptr<IndependentThreadVoid> independentThreadVoid_;
        ConnOnMessage onMessage_;
    };
}


#endif //BASE_CONNECTION_H
