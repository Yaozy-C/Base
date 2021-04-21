#include "Buffer.h"
#include "Log.h"
#include "Tools.h"
#include "DataPacket.h"
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

int main() {
    LOG_INIT_LOGGER();
//    Base::ThreadPool threadPool;
//    A a(0);
//    B b(2);
//    auto f1 = bind(&A::change, &a, ref(b));
//    for (int i = 0; i < 10; ++i) {
//        threadPool.AddTask(f1);
//    }
//    threadPool.Stop();

    RfidDev rfidDev;
    rfidDev.SetJsonText("salkjdlkqwjlkejlqwjlejl2ml1mel21mlemldmsal;mlmaldjlwqjl;ek;lqwke;lqkw;le");


    LOG_ERROR("12321321");

    LOG_DEBUG(to_string(FileSize("Base")));
    return 0;
}