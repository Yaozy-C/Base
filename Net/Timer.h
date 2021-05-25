//
// Created by Yaozy on 2021/5/24.
//

#ifndef BASE_TIMER_H
#define BASE_TIMER_H

#include "Event.h"

namespace Base::Net::Tcp {
    class Timer : public Event {
    public:
        Timer();

        ~Timer();

        void Init();

        void SetTime() const;

        [[nodiscard]] int GetFd() const;

        void Loop() override;

        void SetEvent(const uint32_t &event) override;

    private:
        int fd_;
        int event_;
    };
}

#endif //BASE_TIMER_H
