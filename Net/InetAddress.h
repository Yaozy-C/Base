//
// Created by Yaozy on 2021/5/11.
//

#ifndef BASE_INETADDRESS_H
#define BASE_INETADDRESS_H


#include <netinet/in.h>
#include <string>
#include "SocketOpt.h"

namespace Base::Net::Tcp::Sockets {

    class InetAddress {
    public:
        explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

        InetAddress(const std::string &ip, uint16_t port, bool ipv6 = false);

        explicit InetAddress(const struct sockaddr_in &addr)
                : addr_(addr) {}

        explicit InetAddress(const struct sockaddr_in6 &addr)
                : addr6_(addr) {}

        [[nodiscard]] sa_family_t Family() const { return addr_.sin_family; }

        [[nodiscard]] std::string ToIp() const;

        [[nodiscard]] std::string ToIpPort() const;

        [[nodiscard]] uint16_t ToPort() const;

        [[nodiscard]] const struct sockaddr *GetSockAddr() const {
            return SocketOpt::SockAddrCast(&addr6_);
        }

        void SetSocketAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }

        [[nodiscard]] uint32_t IpNetEndian() const;

        [[nodiscard]] uint16_t PortNetEndian() const { return addr_.sin_port; }

        static bool Resolve(const std::string &hostname, InetAddress *result);

    private:
        union {
            struct sockaddr_in addr_;
            struct sockaddr_in6 addr6_;
        };
    };
}
#endif //BASE_INETADDRESS_H
