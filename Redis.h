//
// Created by Yaozy on 2021/4/12.
//

#ifndef BASE_REDIS_H
#define BASE_REDIS_H
#include <hiredis/hiredis.h>
#include <string>
namespace Base {

    class Redis {

    public:
        Redis(std::string ip,const int &port);
        bool Connect();

        std::string Get(const std::string& key);

        std::string Set(const std::string&key,const std::string& value);

        void DisConnect();
        ~Redis();

    private:
        std::string _ip;
        int _port;
        redisContext* _connect;
        redisReply* _reply;
    };

}

#endif //BASE_REDIS_H
