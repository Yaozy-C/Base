//
// Created by Yaozy on 2021/5/24.
//

#ifndef BASE_TIMER_H
#define BASE_TIMER_H

#include "Event.h"
#include "IndependentThread.h"
#include <vector>
#include <sys/epoll.h>

namespace Base::Thread {
    class TEvent : public Event {
    public:
        TEvent();

        ~TEvent();

        void
        SetTime(const int &second, const int &nSecond, const int &intervalSecond, const int &intervalNSecond);

        [[nodiscard]] int GetFd() const;

        void Loop() override;

        void SetEvent(const uint32_t &event) override;

        void SetCallBack(std::function<void()> func);

        void RegisterRemove(std::function<void(const int &)> disconnect);

    private:
        std::function<void()> _func;
        int _fd;
        int _event;
        bool _interval;
        std::function<void(const int &)> _disconnect;
    };


    class Timer {
    public:
        Timer();

        ~Timer();

        int AddEvent(int fd, const std::shared_ptr<TEvent> &event);

        void WaitLoop();

        int GetSize();

        bool Empty();

        void RemoveEvent(const int &fd);

    private:

        int DELEvent(const int &fd);

        int Wait(int size, std::vector<struct epoll_event> &events, const int &time) const;

        int _fd;
        std::atomic<int> size_;
        std::vector<struct epoll_event> events_;
        std::map<int, std::shared_ptr<TEvent>> connections_;
    };
}

#endif //BASE_TIMER_H
