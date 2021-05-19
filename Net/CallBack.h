//
// Created by Yaozy on 2021/5/18.
//

#ifndef BASE_CALLBACK_H
#define BASE_CALLBACK_H

#include <functional>
#include <memory>
#include "../Public/Buffer.h"

namespace Base {
    namespace Net {
        namespace Tcp {
            class Connection;

            typedef std::function<void(const std::shared_ptr<Connection> &, const std::shared_ptr<Buffer> &)> OnMessage;
            typedef std::function<void(const int &, const std::shared_ptr<Buffer> &)> ConnOnMessage;
            typedef std::function<void(int, int)> DisConnect;

            typedef std::function<int(int, int, int)> EpollMod;

        }
    }
}

#endif //BASE_CALLBACK_H
