#include "Public/DataPacket.h"
#include <chrono>
#include "Timer.h"
#include "Log.h"
using namespace std;

int test(int index, std::chrono::steady_clock::time_point tp) {


    LOG_DEBUG("index:" + std::to_string(index));
    LOG_DEBUG("set:" + std::to_string(tp.time_since_epoch().count()));
    LOG_DEBUG("run:" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    return 0;
}

int main() {
    LOG_INIT_LOGGER();

    try {


        std::shared_ptr<Base::Thread::Timer> timer = std::make_shared<Base::Thread::Timer>();

        std::shared_ptr<Base::Thread::TEvent> event = std::make_shared<Base::Thread::TEvent>();
        timer->AddEvent(event->GetFd(), event);
        std::shared_ptr<Base::Thread::TEvent> event2 = std::make_shared<Base::Thread::TEvent>();
        timer->AddEvent(event2->GetFd(), event2);
//        event->RegisterTimer(timer);

        auto date = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        LOG_DEBUG("post");
        int id = event->AddTask(3000000, true, std::bind(test, 1, date));

        std::this_thread::sleep_for(std::chrono::seconds(22));

        event->Remove(id);
    } catch (const char *msg) {
        LOG_ERROR(msg);
    }

    return 0;
}