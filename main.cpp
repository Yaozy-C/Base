#include <chrono>
#include "Timer.h"
#include "DataPacket.h"


using namespace std;

int test(const int &index) {
    std::cout << index << std::endl;
//    std::cout << hex << &index << std::endl;
    return index;
}


int test2(const std::vector<int> &repeat) {
    for (int i: repeat) {
        std::cout << "repeat:" << i << std::endl;
    }
    std::cout << hex << &repeat << std::endl;

    return 0;
}

int main() {
//    LOG_INIT_LOGGER();

    std::shared_ptr<Base::Thread::Timer> timer = std::make_shared<Base::Thread::Timer>();

    std::shared_ptr<Base::Thread::TEvent> event = std::make_shared<Base::Thread::TEvent>();
    timer->AddEvent(event->GetFd(), event);

    {
        std::vector<int> repeat;
        for (int i = 0; i < 10000; ++i) {
            if (i % 2 == 0) {
                int id = event->AddTask(3000000, true, std::bind(test, i));
                repeat.emplace_back(id);
            } else
                int id = event->AddTask(3000000, false, std::bind(test, i));

        }
        int num = 1;
//        event->AddTask(3000000, false, std::bind(test, num));
//        event->AddTask(3000000, false, std::bind(test2, repeat));
        std::cout << "-----------------" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}