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
#include <iostream>

namespace Base::Thread {


    class Comp {
    public:
        bool operator()(const std::pair<std::chrono::steady_clock::time_point, int> &x,
                        const std::pair<std::chrono::steady_clock::time_point, int> &y) const{
            if (x.second == y.second) {
                return x.second < y.second;
            }
            return x.first.time_since_epoch().count() <= y.first.time_since_epoch().count();
        }
    };

    class IdGen {
    public:
        IdGen() : maxId(0) {}

        void addId(const uint64_t &id) {
            std::unique_lock<std::mutex> lock(mtx);
            idMap[id] = 1;
        }

        uint64_t getId() {
            std::unique_lock<std::mutex> lock(mtx);
            if (idMap.empty()) {
                maxId++;
                //fixme:how to deal with maxId when maxId==uint64 max?
                return maxId;
            } else {
                auto id = idMap.begin()->first;
                idMap.erase(idMap.begin());
                return id;
            }
        }

        uint64_t getMax() {
            std::unique_lock<std::mutex> lock(mtx);
            return maxId;
        }

    private:
        std::unordered_map<uint64_t, int> idMap;
        std::mutex mtx;
        uint64_t maxId;
    };

    class Task {
    public:
        Task(const uint64_t &id, const std::chrono::microseconds &milliseconds, const bool &repeat, std::function<void()> _task);

         std::chrono::microseconds Valid(std::chrono::steady_clock::time_point tp) const ;

        void SetTimePoint(std::chrono::steady_clock::time_point tp);

        std::chrono::microseconds _microseconds;
        std::atomic<bool> _repeat;
        std::atomic<bool> _run;
        uint64_t _id;
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

        uint64_t AddTask(const std::chrono::microseconds &microseconds, const bool &repeat, const std::function<void()> &func);

        void RegisterTimer(const std::shared_ptr<Timer> &timer);

        void RegisterThread(const std::shared_ptr<EventLoop> &thread);

        void Remove(const uint64_t &index);

    private:
        int _fd;
        int _event;
        std::map<uint64_t, std::shared_ptr<Task>> _tasks;
        std::set<std::pair<std::chrono::steady_clock::time_point, uint64_t>, Comp> _taskList;
        std::shared_ptr<EventLoop> _thread;
        std::shared_ptr<EventLoop> _worker;
        std::weak_ptr<Timer> _timer;
        IdGen gen;

        void ResetTask(const std::vector<std::shared_ptr<Task>> &tasks);

        void SetTime(const std::chrono::microseconds &second) const;

        void Run(const std::vector<std::shared_ptr<Task>> &tasks);

        void AddTaskInLoop(const std::shared_ptr<Task> &task);

        void RemoveTaskInLoop(const uint64_t &index);
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

        void Remove();

        int _fd;
        std::atomic<int> _size;
        std::atomic<bool> _init;
        std::vector<struct epoll_event> _events;
        std::map<int, std::shared_ptr<TEvent>> _connections;
        std::shared_ptr<EventLoop> _thread;
        std::atomic<bool> _run;

        void RemoveEventInLoop(const int &fd);

        int AddEventInLoop(int fd, const std::shared_ptr<TEvent> &event);
    };
}

#endif //BASE_TIMER_H
