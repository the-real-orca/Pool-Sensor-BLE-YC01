#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

struct sensorReadings_t
{
    uint8_t type;
    time_t time;
    int16_t rssi;
    float pH;
    float ec;
    float salt;
    float tds;
    float orp;
    float cl;
    float temp;
    float bat;
};

bool compareBLEAddress(const NimBLEAddress& address, const char* targetAddress);
bool compareBLEAddress(const NimBLEAddress& address, const String& targetAddress);

class BLE_YC01 {
public:

    static std::vector<NimBLEAddress> scan();

    BLE_YC01(NimBLEAddress const& addr, String const& name = "");
    NimBLEAddress getAddress() const { return address; }
    String getName() const { return name; }
    void setName(String const& name) { this->name = name; }
    String getSensorType() const { return sensorType; }
    sensorReadings_t getReadings() const { return readings; }
    bool readData();

protected:
    NimBLEAddress address; // Address of the BLE device
    String sensorType;
    String name;
    sensorReadings_t readings;
};
