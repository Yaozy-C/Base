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

int main() {
    LOG_INIT_LOGGER();

    Base::Net::Tcp::Sockets::InetAddress inetAddress(4567);
    Base::Net::Tcp::TcpServer tcpServer(inetAddress);
    tcpServer.Start();
    std::this_thread::sleep_for(std::chrono::seconds(2000000));
    return 0;
}