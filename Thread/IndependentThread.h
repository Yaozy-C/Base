//
// Created by Yaozy on 2020/9/21.
//

#ifndef BASE_INDEPENDENTTHREAD_H
#define BASE_INDEPENDENTTHREAD_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>
#include <set>
#include <map>
#include "SafeQue.h"

namespace Base {
    template<typename T>
    class IndependentThread {
    private:
        std::thread _thread;
        std::mutex mtx;
        std::condition_variable _cv;
        std::vector<T> _tasks;
        std::atomic<bool> _shutdown;
    public:

        IndependentThread() : _shutdown(false) {
            _thread = std::thread(&IndependentThread::Execute, this);
        };

        ~IndependentThread() = default;

        virtual void Execute() {
            while (!_shutdown) {
                {
                    std::unique_lock<std::mutex> _lock(mtx);
                    while (_tasks.empty()) {
                        if (_shutdown) {
                            return;
                        }
                        _cv.wait(_lock);
                    }
                }

                std::vector<T> tasks;
                {
                    std::unique_lock<std::mutex> _lock(mtx);
                    _tasks.swap(tasks);
                }

                for (int i = 0; i < tasks.size(); ++i) {
                    Run(tasks[i]);
                }
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        };

        void Shutdown() {
            while (!_tasks.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            _shutdown = true;
            _cv.notify_all();
            if (_thread.joinable())
                _thread.join();
        };

        void AddTask(T &task) {
            {
                std::unique_lock<std::mutex> _lock(mtx);
                _tasks.emplace_back(task);
            }
            _cv.notify_all();
        };

    protected:
        virtual void Run(T &task) = 0;
    };

    class IndependentThreadVoid {
    private:
        std::thread _thread;
        std::mutex mtx;
        std::condition_variable _cv;
        std::atomic<bool> _shutdown;
        std::vector<std::function<void()>> _tasks;
    public:

        IndependentThreadVoid() : _shutdown(false) {
            _thread = std::thread(&IndependentThreadVoid::Execute, this);
        };

        ~IndependentThreadVoid() = default;

        virtual void Execute() {
            while (!_shutdown) {
                {
                    std::unique_lock<std::mutex> _lock(mtx);
                    while (_tasks.empty()) {
                        if (_shutdown) {
                            return;
                        }
                        _cv.wait(_lock);
                    }
                }

                std::vector<std::function<void()>> tasks;
                {
                    std::unique_lock<std::mutex> _lock(mtx);
                    _tasks.swap(tasks);
                }
                for (int i = 0; i < tasks.size(); ++i) {
                    tasks[i]();
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds (1));
            }
        };

        void Shutdown() {
            while (!_tasks.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
            _shutdown = true;
            _cv.notify_all();
            if (_thread.joinable())
                _thread.join();
        };

        template<class F, class...Args>
        void AddTask(F &&f, Args &&... args) {
            auto ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            std::function<void()> func = [ptr]() {
                (*ptr)();
            };
            {
                std::unique_lock<std::mutex> lock(mtx);
                _tasks.emplace_back(func);
            }
            _cv.notify_one();
        }
    };

    class Comp {
    public:
        bool operator()(const std::pair<std::chrono::steady_clock::time_point, int> &x,
                        const std::pair<std::chrono::steady_clock::time_point, int> &y) {

            return x.first.time_since_epoch().count() < y.first.time_since_epoch().count();
        }
    };

    class IndependentThreadTimeLoop {
    private:
        std::thread _thread;
        std::mutex mtx_;
        std::condition_variable _cv;

        std::atomic<bool> _shutdown;
        std::set<std::pair<std::chrono::steady_clock::time_point, int>, Comp> timer;
        std::chrono::microseconds time;
        std::atomic<int> index;
        std::map<int, std::function<void()>> _tasks;
    public:

        IndependentThreadTimeLoop(int microseconds) : _shutdown(false), time(microseconds), index(0) {
            _thread = std::thread(&IndependentThreadTimeLoop::Execute, this);
        };

        ~IndependentThreadTimeLoop() {
            Shutdown();
        };

        virtual void Execute() {
            while (!_shutdown) {
                std::unique_lock<std::mutex> _lock(mtx_);
                while (_tasks.empty()) {
                    if (_shutdown) {
                        return;
                    }
                    _cv.wait(_lock);
                }

                auto now = std::chrono::steady_clock::now();

                for (auto iter = timer.begin(); iter != timer.end();) {
                    if ((now - iter->first).count() > 0) {
                        _tasks[iter->second]();
                        _tasks.erase(iter->second);
                        iter = timer.erase(iter);
                    } else
                        break;
                }
            }
        };

        void Shutdown() {
            _shutdown = true;
            _cv.notify_all();
            if (_thread.joinable())
                _thread.join();
        };


        template<class F, class...Args>
        void AddTaskAt(F &&f, Args &&... args) {
            auto ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            std::function<void()> func = [ptr]() {
                (*ptr)();
            };

            auto data = std::pair<std::chrono::steady_clock::time_point, int>(
                    std::chrono::steady_clock::now() + time, index);

            auto pair = timer.insert(data);
            if (!pair.second) {
                throw "AddTaskAt error";
            }
            _tasks[index] = func;
            index++;
            if (index == 10000000)
                index = 0;
            _cv.notify_one();
        }
    };
}


#endif //BASE_INDEPENDENTTHREAD_H
