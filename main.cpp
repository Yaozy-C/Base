#include <chrono>
#include "Timer.h"
#include "DataPacket.h"
#include "Log.h"

using namespace std;

void TTTT(const std::chrono::milliseconds &time) {
    DEBUG << time / std::chrono::seconds(1);
}

int main() {

    std::string a = "123";
    TRACE << a << 1 << "3123";
    DEBUG << a << 1 << "3123";
    INFO << a << 1 << "3123";
    WARN << a << 1 << "3123";
    ERROR << a << 1 << "3123";

    auto tp = std::chrono::seconds(10) + std::chrono::milliseconds(20);

    DEBUG << tp / std::chrono::seconds(1);

    DEBUG << tp / std::chrono::milliseconds(1);

    TTTT(std::chrono::seconds(20));

    auto timer = std::make_shared<Base::Thread::Timer>();
    auto event = std::make_shared<Base::Thread::TEvent>();

    timer->AddEvent(event->GetFd(), event);
    event->AddTask(std::chrono::seconds(2), false ,[]() {
        DEBUG << 1;
    });
    event->AddTask(std::chrono::milliseconds (100), false ,[]() {
        DEBUG << 2;
    });
    event->AddTask(std::chrono::microseconds (100), false ,[]() {
        DEBUG << 3;
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}