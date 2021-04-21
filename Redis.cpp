//
// Created by Yaozy on 2021/4/12.
//

#include "Redis.h"
#include "Log.h"
#include <utility>


namespace Base {

    Redis::Redis(std::string  ip, const int &port):_ip(std::move(ip)),_port(port),_connect(nullptr),_reply(nullptr) {
    }

    bool Redis::Connect() {
        _connect = ::redisConnect(_ip.c_str(), _port);
        if (this->_connect != nullptr && this->_connect->err) {
            printf("connect error: %s\n", this->_connect->errstr);
            LOG_ERROR("Redis Connect err:"+std::string(this->_connect->errstr));
            return false;
        }
        return true;
    }

    void Redis::DisConnect() {
        if (_connect!= nullptr)
            ::redisFree(_connect);
    }

    Redis::~Redis() =default;

}