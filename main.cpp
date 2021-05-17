

#include <set>
#include "Public/Buffer.h"
#include "Public/Log.h"
#include "Public/DataPacket.h"
#include "Net/TcpServer.h"


using namespace std;


class RfidDev : public Base::DataPacket {
public:
    RfidDev();

    RfidDev(const RfidDev &rfidDev);

    RfidDev &operator=(const RfidDev &rfidDev);

    ~RfidDev();

    void DecodeJson(cJSON *reader) override;

    void EncodeJson(cJSON *writer) override;

    string id;
    string name;
    vector<string> ip;
    int port;
};


RfidDev::RfidDev() {
    port = 0;
}

RfidDev::RfidDev(const RfidDev &rfidDev) {
    id = rfidDev.id;
    name = rfidDev.name;
    ip = rfidDev.ip;
    port = rfidDev.port;
}

RfidDev &RfidDev::operator=(const RfidDev &rfidDev) {
    if (this == &rfidDev)
        return *this;
    id = rfidDev.id;
    name = rfidDev.name;
    ip = rfidDev.ip;
    port = rfidDev.port;
    return *this;
}

RfidDev::~RfidDev() = default;

void RfidDev::DecodeJson(cJSON *reader) {
    Base::GetJsonValue(reader, "ID", id);
    Base::GetJsonValue(reader, "Name", name);
    Base::GetJsonValue(reader, "IP", ip);
    Base::GetJsonValue(reader, "Port", port);
}

void RfidDev::EncodeJson(cJSON *writer) {
    Base::AddJsonValue(writer, "ID", id);
    Base::AddJsonValue(writer, "Name", name);
    Base::AddJsonValue(writer, "IP", ip);
    Base::AddJsonValue(writer, "Port", port);
}




//Base::IndependentThreadTimeLoop timer(100);
//
//void test(){
//    LOG_DEBUG("123");
//
//    timer.AddTaskAt(&test);
//}
//
//void test2() {
//    LOG_DEBUG("456");
//
//    timer.AddTaskAt(&test2);
//}





int main() {
    LOG_INIT_LOGGER();

//    auto time = std::chrono::milliseconds(10);
//
//    auto last = std::chrono::steady_clock::now();
//
//    std::set<std::pair<std::chrono::steady_clock::time_point, int>,Base::Comp> timer;
//
//    auto pair = timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 1
//    ));
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 2
//    ));
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 3
//    ));
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 4
//    ));
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 5
//    ));
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 6
//    ));
//
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    timer.insert(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now() + time, 7
//    ));
//
//
//    auto iter = timer.lower_bound(std::pair<std::chrono::steady_clock::time_point, int>(
//            std::chrono::steady_clock::now()+time, 8));
//
//    LOG_DEBUG(std::to_string(timer.size()));
//    for ( auto begin = timer.begin(); begin != iter; ) {
//        LOG_DEBUG(std::to_string(begin->second));
//        begin = timer.erase(begin);
//    }
//
//    LOG_DEBUG(std::to_string(timer.size()) + "     "+std::to_string(iter->second));
//

    Base::Net::Tcp::Sockets::InetAddress inetAddress(4567);
    Base::Net::Tcp::TcpServer tcpServer(inetAddress);
    tcpServer.Start();
    std::this_thread::sleep_for(std::chrono::seconds(2000000));


    return 0;
}