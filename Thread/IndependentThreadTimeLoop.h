//
// Created by zyyao3 on 2021/10/18.
//

#ifndef BASE_INDEPENDENTTHREADTIMELOOP_H
#define BASE_INDEPENDENTTHREADTIMELOOP_H

#include <mutex>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>
#include <map>
#include "Timer.h"
#include "Log.h"


namespace Base::Thread {

    class IndependentThreadTimeLoop {
    private:
        std::thread _thread;
        std::mutex _mtx;
        std::condition_variable _cv;

        std::atomic<bool> _shutdown;
        Timer _timer;
        std::shared_ptr<TEvent> tEvent;

    public:

        IndependentThreadTimeLoop() : _shutdown(false) {
            tEvent = std::make_shared<TEvent>();
            tEvent->SetTime(2, 0, 2, 0);

            tEvent->RegisterRemove(std::bind(&Timer::RemoveEvent, &_timer, std::placeholders::
            _1));

            int res = _timer.AddEvent(tEvent->GetFd(), tEvent);
            if (res)
                LOG_ERROR("AddEvent error");

            _thread = std::thread(&IndependentThreadTimeLoop::Execute, this);
        };

        ~IndependentThreadTimeLoop() {
            while (!_timer.Empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
            _shutdown = true;
            _cv.notify_one();
            if (_thread.joinable())
                _thread.join();

            _timer.RemoveEvent(tEvent->GetFd());
        };

        void CancelTask(const int &fd) {
            _timer.RemoveEvent(fd);
        }

        template<class F, class...Args>
        int AddTask(const int &second, const int &nSecond, const int &intervalSecond,
                    const int &intervalNSecond, F &&f, Args &&... args) {
            std::function<void()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            std::shared_ptr<TEvent> event = std::make_shared<TEvent>();
            event->SetTime(second, nSecond, intervalSecond, intervalNSecond);
            event->SetCallBack(func);

            event->RegisterRemove(std::bind(&Timer::RemoveEvent, &_timer, std::placeholders::_1));

//            {
//                std::unique_lock<std::mutex> _lock;

            int res = _timer.AddEvent(event->GetFd(), event);
            if (res)
                LOG_ERROR("AddEvent error");
//            }

            _cv.notify_one();

            return event->GetFd();
        }

    private:
        void Execute() {
            while (!_shutdown) {
                {
                    std::unique_lock<std::mutex> _lock(_mtx);
                    while (_timer.Empty()) {
                        if (_shutdown) {
                            return;
                        }
                        _cv.wait(_lock);
                    }
                }
                _timer.WaitLoop();
            }
        };
    };
}
#endif //BASE_INDEPENDENTTHREADTIMELOOP_H
