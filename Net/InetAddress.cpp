//
// Created by Yaozy on 2021/5/11.
//

#include <netdb.h>
#include "InetAddress.h"
#include "SocketOpt.h"
#include "../Public/Log.h"

using namespace Base::Net::Tcp::SocketOpt;
using namespace Base::Net::Tcp::Sockets;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    if (ipv6) {
        MemZero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = htobe16(port);
    } else {
        MemZero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = htobe32(ip);
        addr_.sin_port = htobe16(port);
    }
}

InetAddress::InetAddress(const std::string &ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        MemZero(&addr6_, sizeof addr6_);
        FromIpPort(ip.c_str(), port, &addr6_);
    } else {
        MemZero(&addr_, sizeof addr_);
        FromIpPort(ip.c_str(), port, &addr_);
    }
}

std::string InetAddress::ToIp() const {
    char buf[64] = "";
    SocketOpt::ToIp(buf, sizeof buf, GetSockAddr());
    return buf;
}

uint16_t InetAddress::ToPort() const {
    return htobe16(PortNetEndian());
}

std::string InetAddress::ToIpPort() const {
    char buf[64] = "";
    SocketOpt::ToIpPort(buf, sizeof buf, GetSockAddr());
    return buf;
}

uint32_t InetAddress::IpNetEndian() const {
    return addr_.sin_addr.s_addr;
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::Resolve(const std::string &hostname, InetAddress *out) {
    struct hostent hent{};
    struct hostent *he = nullptr;
    int herrno = 0;
    MemZero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != nullptr) {
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr *>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_ERROR("InetAddress::Resolve " + std::to_string(ret));
        }
        return false;
    }
}
