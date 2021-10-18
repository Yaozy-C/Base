#include <IndependentThreadTimeLoop.h>
#include "Public/DataPacket.h"

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

int test(int index, std::chrono::steady_clock::time_point tp) {


    LOG_DEBUG("index:" + std::to_string(index));
    LOG_DEBUG("set:" + std::to_string(tp.time_since_epoch().count()));
    LOG_DEBUG("run:" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    return 0;
}

int main() {
    LOG_INIT_LOGGER();

    try {

        Base::Thread::IndependentThreadTimeLoop timer;

        auto date = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        LOG_DEBUG("post");
        int task = timer.AddTask(20, 0, 3, 0, std::bind(test, 1, date), date);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        timer.CancelTask(task);
    } catch (const char *msg) {
        LOG_ERROR(msg);
    }

    return 0;
}