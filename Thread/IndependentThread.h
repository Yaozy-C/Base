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

namespace Base {
    template<typename T>
    class IndependentThread {
    private:
        std::thread _thread;
        std::mutex mtx;
        std::condition_variable _cv;
        std::vector<T> _tasks;
        std::atomic<bool> _shutdown;
        std::atomic<bool> _run;
    public:

        IndependentThread() : _shutdown(false), _run(false) {
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

                _run = true;
                for (int i = 0; i < tasks.size(); ++i) {
                    Run(tasks[i]);
                }
                _run = false;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        };

        void Shutdown() {
            while (!_tasks.empty() || _run) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            _shutdown = true;
            _cv.notify_one();
            if (_thread.joinable())
                _thread.join();
        };

        void AddTask(T &task) {
            {
                std::unique_lock<std::mutex> _lock(mtx);
                _tasks.emplace_back(task);
            }
            _cv.notify_one();
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
        std::atomic<bool> _run;
    public:

        IndependentThreadVoid() : _shutdown(false), _run(false) {
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

                _run = true;
                for (auto &task: tasks) {
                    task();
                }
                _run = false;
            }
        };

        void Shutdown() {
            while (!_tasks.empty() || _run) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
            _shutdown = true;
            _cv.notify_one();
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
        std::mutex _mtx;
        std::condition_variable _cv;

        std::atomic<bool> _shutdown;
        std::set<std::pair<std::chrono::steady_clock::time_point, int>, Comp> timer;
        std::atomic<int> _index;
        std::map<int, std::function<void()>> _tasks;
    public:

        IndependentThreadTimeLoop() : _shutdown(false), _index(0) {
            _thread = std::thread(&IndependentThreadTimeLoop::Execute, this);
        };

        ~IndependentThreadTimeLoop() {
            Shutdown();
        };

        virtual void Execute() {
            while (!_shutdown) {
                {
                    std::unique_lock<std::mutex> _lock(_mtx);
                    while (_tasks.empty()) {
                        if (_shutdown) {
                            return;
                        }
                        _cv.wait(_lock);
                    }
                }

                std::vector<std::function<void()>> tasks;
                {
                    std::unique_lock<std::mutex> _lock(_mtx);
                    auto iter = timer.lower_bound(std::pair<std::chrono::steady_clock::time_point, int>(
                            std::chrono::steady_clock::now(), 0));
                    for (auto it = timer.begin(); it != iter;) {
                        tasks.emplace_back(_tasks[it->second]);
                        _tasks.erase(it->second);
                        it = timer.erase(it);
                    }
                }

                for (auto &task: tasks) {
                    task();
                }

            }
        };

        void Shutdown() {
            while (!_tasks.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
            _shutdown = true;
            _cv.notify_one();
            if (_thread.joinable())
                _thread.join();
        };


        template<class F, class...Args>
        void AddTaskAt(F &&f, std::chrono::steady_clock::time_point time, Args &&... args) {
            auto ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            std::function<void()> func = [ptr]() {
                (*ptr)();
            };

            auto data = std::pair<std::chrono::steady_clock::time_point, int>(
                    time, _index);

            auto pair = timer.insert(data);
            if (!pair.second) {
                throw "AddTaskAt error";
            }
            _tasks[_index] = func;
            _index++;
            if (_index == 10000000)
                _index = 0;
            _cv.notify_one();
        }
    };
}


#endif //BASE_INDEPENDENTTHREAD_H
