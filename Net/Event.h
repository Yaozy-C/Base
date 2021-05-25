//
// Created by Yaozy on 2021/5/24.
//

#ifndef BASE_EVENT_H
#define BASE_EVENT_H

#include <cstdint>

namespace Base::Net::Tcp {
    class Event {
    public:
        virtual void Loop() = 0;

        virtual void SetEvent(const uint32_t &) = 0;
    };
}


#endif //BASE_EVENT_H
