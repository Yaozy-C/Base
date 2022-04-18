//
// Created by Yaozy on 2021/5/24.
//

#ifndef BASE_TIMER_H
#define BASE_TIMER_H

#include "Event.h"
#include "IndependentThread.h"
#include <vector>
#include <set>
#include <sys/epoll.h>

namespace Base::Thread {


    class Comp {
    public:
        bool operator()(const std::pair<std::chrono::steady_clock::time_point, int> &x,
                        const std::pair<std::chrono::steady_clock::time_point, int> &y) const{

            return x.first.time_since_epoch().count() < y.first.time_since_epoch().count();
        }
    };

    class Task {
    public:
        Task(const int &id, const int &microseconds, const bool &repeat, std::function<void()> _task);

        [[nodiscard]] int Valid(std::chrono::steady_clock::time_point tp) const;

        void SetTimePoint(std::chrono::steady_clock::time_point tp);

        int _microseconds;
        std::atomic<bool> _repeat;
        int _id;
        std::function<void()> _func;
        std::chrono::steady_clock::time_point _tp;
    };

    class Timer;

    class TEvent : public Event {
    public:
        TEvent();

        ~TEvent();

        [[nodiscard]] int GetFd() const;

        void Loop() override;

        void SetEvent(const uint32_t &event) override;

        int AddTask(const int &microseconds, const bool &repeat, const std::function<void()> &func);

        void RegisterTimer(const std::shared_ptr<Timer> &timer);

        void RegisterThread(const std::shared_ptr<EventLoop> &thread);

        void Remove(const int &index);

    private:
        int _fd;
        std::atomic<int> _id;
        int _event;
        std::map<int, std::shared_ptr<Task>> _tasks;
        std::set<std::pair<std::chrono::steady_clock::time_point, int>, Comp> _taskList;
        std::shared_ptr<EventLoop> _thread;
        std::shared_ptr<EventLoop> _worker;
        std::weak_ptr<Timer> _timer;

        void ResetTask(const std::vector<std::shared_ptr<Task>> &tasks);

        void SetTime(const int &second) const;

        void Run(const std::vector<std::shared_ptr<Task>> &tasks);

        void AddTaskInLoop(const std::shared_ptr<Task> &task);

        void RemoveTaskInLoop(const int &index);
    };


class Timer :public std::enable_shared_from_this<Timer>{
    public:
        Timer();

        ~Timer();

        void AddEvent(int fd, const std::shared_ptr<TEvent> &event);

        void WaitLoop();

        void RemoveEvent(const int &fd);

    private:

        int DELEvent(const int &fd);

        int Wait(int size, std::vector<struct epoll_event> &events, const int &time) const;

        int _fd;
        std::atomic<int> _size;
        std::vector<struct epoll_event> _events;
        std::map<int, std::shared_ptr<TEvent>> _connections;
        std::shared_ptr<EventLoop> _thread;

        void RemoveEventInLoop(const int &fd);

        int AddEventInLoop(int fd, const std::shared_ptr<TEvent> &event);
    };
}

#endif //BASE_TIMER_H
