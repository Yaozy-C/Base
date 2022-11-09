//
// Created by Yaozy on 2021/5/24.
//

#include "Timer.h"
#include "Log.h"
#include "SocketOpt.h"
#include <sys/timerfd.h>
#include <iostream>

using namespace Base::Thread;


Task::Task(const int &id, const int &microseconds, const bool &repeat, std::function<void()> func) : _microseconds(
        microseconds), _repeat(repeat), _id(id), _run(true) {
    _func = std::move(func);
}

int Task::Valid(std::chrono::steady_clock::time_point tp) const {

    std::chrono::duration<int, std::ratio<1, 1000000>> time_span = std::chrono::duration_cast<std::chrono::duration<int,
            std::ratio<1, 1000000>>>(
            _tp - tp);
    return time_span.count();
}

void Task::SetTimePoint(std::chrono::steady_clock::time_point tp) {
    _tp = tp;
}


TEvent::TEvent() : _fd(-1), _event(0), _id(-1) {
    _fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (_fd < 0) {
        abort();
    }

    _worker = std::make_shared<EventLoop>();
}

TEvent::~TEvent() {
    if (!_timer.expired())
        _timer.lock()->RemoveEvent(_fd);
    ::close(_fd);
}

int TEvent::GetFd() const {
    return _fd;
}

void TEvent::SetTime(const int &microseconds) const {
    struct itimerspec new_value{}, old{};
    int second = microseconds / 1000000;
    int nsecond = microseconds - second * 1000000;
    if (nsecond <= 0) {
        if (second <= 0)
            nsecond = 1;
    }
//    std::cout << second << "                " << nsecond << std::endl;
    new_value.it_value.tv_sec = second;
    new_value.it_value.tv_nsec = nsecond * 1000;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;
    timerfd_settime(_fd, 0, &new_value, &old);
}

void TEvent::Loop() {
    uint64_t exp = 0;
    size_t res = read(_fd, &exp, sizeof(uint64_t));
    if (res < 0)
        LOG_ERROR("Timer::Loop()");

    std::vector<std::shared_ptr<Task>> tasks;
    auto iter = _taskList.lower_bound(std::pair<std::chrono::steady_clock::time_point, int>(
            std::chrono::steady_clock::now(), 0));
    for (auto it = _taskList.begin(); it != iter;) {
        if (_tasks[it->second]->_run)
            tasks.emplace_back(_tasks[it->second]);
        it = _taskList.erase(it);
    }
    _worker->AddTask([this, tasks] { Run(tasks); });

    ResetTask(tasks);
}

void TEvent::Run(const std::vector<std::shared_ptr<Task>> &tasks) {
    for (auto &task: tasks) {
        task->_func();
    }
}

void TEvent::SetEvent(const uint32_t &event) {
    _event = event;
}

int TEvent::AddTask(const int &microseconds, const bool &repeat, const std::function<void()> &func) {
    int id = 0;
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _id++;
        id = _id;

    }
    std::shared_ptr<Task> task = std::make_shared<Task>(id, microseconds, repeat, func);
    _thread->AddTask([this, task] { AddTaskInLoop(task); });
    return id;
}

void TEvent::AddTaskInLoop(const std::shared_ptr<Task> &task) {
    int id = task->_id;
    _tasks[id] = task;
    auto time = std::chrono::steady_clock::now() + std::chrono::microseconds(task->_microseconds);
    auto date = std::pair<std::chrono::steady_clock::time_point, int>(time, id);

    _tasks[id]->SetTimePoint(time);

    if (_taskList.empty()) {
        SetTime(task->_microseconds);
    } else if (time.time_since_epoch().count() < _taskList.begin()->first.time_since_epoch().count()) {
        std::chrono::duration<int, std::ratio<1, 1000000>>
                time_span = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<1, 1000000>>>(
                time - std::chrono::steady_clock::now());
        SetTime(time_span.count());
    }

    auto pair = _taskList.insert(date);
    if (!pair.second) {
        throw std::runtime_error("AddTaskAt error");
    }
}

void TEvent::RegisterTimer(const std::shared_ptr<Timer> &timer) {
    _timer = timer;
}

void TEvent::RegisterThread(const std::shared_ptr<EventLoop> &thread) {
    _thread = thread;
}


void TEvent::Remove(const int &index) {
    _thread->AddTask([this, index] { RemoveTaskInLoop(index); });
}

void TEvent::RemoveTaskInLoop(const int &index) {
    auto iter = _tasks.find(index);
    if (iter != _tasks.end()) {
        if (iter->second->_run) {
            iter->second->_run = false;
            iter->second->_repeat = false;
        }
    }
}

void TEvent::ResetTask(const std::vector<std::shared_ptr<Task>> &tasks) {
    auto time = std::chrono::steady_clock::now();

    for (auto &task: tasks) {
        if (task->_repeat) {
            auto rtime = time + std::chrono::microseconds(task->_microseconds);
            task->SetTimePoint(rtime);

            auto date = std::pair<std::chrono::steady_clock::time_point, int>(rtime, task->_id);

//            auto pair =
            _taskList.insert(date);
//            if (!pair.second) {
//                throw "AddTaskAt error";
//            }
        } else
            _tasks.erase(_tasks.find(task->_id));
    }

    int microseconds = 10000000;
    if (!_taskList.empty())
        microseconds = _tasks[_taskList.begin()->second]->Valid(time);

    SetTime(microseconds);
}


Timer::Timer() : _fd(-1), _size(0), _init(false) {
    _fd = epoll_create(256);
    _events.resize(16);
    _thread = std::make_shared<EventLoop>();
}

Timer::~Timer() {
    auto fd = _fd;
    _thread->AddTask([fd] {
        ::close(fd);
    });
}

void Timer::AddEvent(int fd, const std::shared_ptr<TEvent> &event) {
    event->RegisterThread(_thread);
    event->RegisterTimer(shared_from_this());
    _thread->AddTask([this, fd, event] { AddEventInLoop(fd, event); });
    if (!_init) {
        _thread->AddTask([this] { WaitLoop(); });
        _init = true;
    }
}

int Timer::AddEventInLoop(int fd, const std::shared_ptr<TEvent> &event) {
    auto iter = _connections.find(fd);
    if (iter != _connections.end()) {
        LOG_ERROR("event is existence");
        return 0;
    } else
        _connections[event->GetFd()] = event;
    struct epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    int res = epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &ev);
    if (res >= 0) {
        _size++;
        if (_size > _events.size())
            _events.resize(_events.size() * 1.5);
    } else {
        LOG_DEBUG(strerror(errno));
        _connections.erase(fd);
    }
    return res;
}

int Timer::DELEvent(const int &fd) {
    struct epoll_event ev{};
    ev.data.fd = fd;

    int res = epoll_ctl(_fd, EPOLL_CTL_DEL, fd, &ev);
    if (res >= 0)
        _size--;
    return res;
}

void Timer::RemoveEvent(const int &fd) {
    _thread->AddTask([this, fd] { RemoveEventInLoop(fd); });
}

void Timer::RemoveEventInLoop(const int &fd) {
    auto iter = _connections.find(fd);
    if (iter != _connections.end()) {
        int res = DELEvent(fd);
        if (res < 0)
            LOG_ERROR("remove fd error");
        _connections.erase(fd);
    }
}

int Timer::Wait(int size, std::vector<struct epoll_event> &events, const int &time) const {
    return epoll_wait(_fd, &*events.begin(), size, time);
}

void Timer::WaitLoop() {
    int num = Wait(_size, _events, 100);
//    std::cout << "num:" << num << "    size:" << std::endl;
    for (int i = 0; i < num; ++i) {
        auto cn = _connections[_events[i].data.fd];
        cn->SetEvent(_events[i].events);
        cn->Loop();
    }
    _thread->AddTask([this] { WaitLoop(); });
}