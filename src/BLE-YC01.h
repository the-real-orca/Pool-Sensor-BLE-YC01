#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

struct sensorReadings_t
{
    uint8_t type;
    time_t time;
    float pH;
    float ec;
    float salt;
    float tds;
    float orp;
    float cl;
    float temp;
    float bat;
};


class BLE_YC01 {
public:

    static std::vector<NimBLEAddress> scan();

    BLE_YC01(NimBLEAddress const& addr);
    NimBLEAddress getAddress() const { return address; }
    String getName() const { return name; }
    String getSensorType() const { return sensorType; }
    sensorReadings_t getReadings() const { return readings; }
    bool readData();

protected:
    NimBLEAddress address; // Address of the BLE device
    String sensorType;
    String name;
    sensorReadings_t readings;
};
