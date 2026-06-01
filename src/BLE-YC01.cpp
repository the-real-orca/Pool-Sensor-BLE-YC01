#include <Arduino.h>
#include <NimBLEDevice.h>
#include "esp_task_wdt.h"

#include "config.h"
#include "BLE-YC01.h"


// BLE device configuration
std::string targetName = "BLE-YC01";
NimBLEUUID serviceUUID("0000ff01-0000-1000-8000-00805f9b34fb");
NimBLEUUID charUUID("0000ff02-0000-1000-8000-00805f9b34fb");


bool compareBLEAddress(const NimBLEAddress& address, const char* targetAddress) {
    std::string addr1 = address.toString();
    std::string addr2 = targetAddress;

    // convert to lower case
    std::transform(addr1.begin(), addr1.end(), addr1.begin(), ::tolower);
    std::transform(addr2.begin(), addr2.end(), addr2.begin(), ::tolower);

    return addr1 == addr2;
}

bool compareBLEAddress(const NimBLEAddress& address, const String& targetAddress) {
    return compareBLEAddress(address, targetAddress.c_str());
}

/**
 * @brief Helper function to convert 2 bytes to int16 (big endian)
 * @param data Data array
 * @param idx Starting index
 * @return 16-bit integer
 */
static int16_t toInt16(uint8_t const data[], int idx) {
    return ((uint16_t)data[idx] << 8) | (uint16_t)data[idx + 1];
}

/**
 * @brief Decodes the proprietary BLE data from the sensor
 * @param data Pointer to raw data
 * @param length Length of data
 * @param output Buffer (size >= length) to write decoded data into
 * @return true on success, false on invalid length
 */
static bool decodeData(uint8_t const data[], int length, uint8_t output[]) {
    if (length < 2 || length > 60) {
        DEBUG_println("decodeData: invalid length");
        return false;
    }

    uint8_t tmp, hibit0, lobit0, hibit1, lobit1;
    tmp=data[length-1];
    for(int i=length-1;i>0;i--)
    {
        hibit1 =( tmp&0x55) << 1;
        lobit1 = (tmp&0xAA) >> 1;
        tmp=data[i-1];
        hibit0 = (tmp&0x55) << 1;
        lobit0 = (tmp&0xAA) >> 1;

        output[i] = ~(hibit1|lobit0);
        tmp = ~(hibit0|lobit1);
        output[i-1] = tmp;
    }
    return true;
}

/**
 * @brief Simple XOR checksum calculation
 * @param data Data pointer
 * @param length Data length
 * @return Checksum result
 */
static uint8_t checksum(const uint8_t* data, int length)		
{		
    uint8_t i = 0;	
    uint8_t chksum = 0;	
    for(i = 0; i < length; i ++)	
        chksum = chksum ^ data[i];
    return chksum;	
}


static std::vector<NimBLEAddress> foundDevices;
static bool scanningActive = false;

class MyScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice *device) {
        String addr = device->getAddress().toString().c_str();
        String name = device->getName().c_str();
        bool hasService = device->isAdvertisingService(serviceUUID);
        int rssi = device->getRSSI();

        Serial.printf("BLE[%s] name=\"%s\" rssi=%d service=%d\n",
                      addr.c_str(), name.c_str(), rssi, hasService);

        // Collect all devices; YC01 may not advertise its service UUID
        bool alreadyFound = false;
        for (const auto& a : foundDevices) {
            if (a == device->getAddress()) {
                alreadyFound = true;
                break;
            }
        }
        if (!alreadyFound) {
            foundDevices.push_back(device->getAddress());
        }
    }

    void onScanEnd(const NimBLEScanResults &results, int reason) {
        scanningActive = false;
        Serial.printf("Scan complete. reason=%d total=%d candidates=%d\n",
                      reason, results.getCount(), foundDevices.size());
    }
};

static MyScanCallbacks scanCallbacks;

bool BLE_YC01::startScan(uint32_t durationMs) {
    if (scanningActive) return false;
    
    foundDevices.clear();
    scanningActive = true;
    
    NimBLEScan *pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(&scanCallbacks);
    pScan->setInterval(45);
    pScan->setWindow(15);
    pScan->setActiveScan(true);
    
    // NimBLE 2.5: start() passes duration directly to ble_gap_disc (expects milliseconds)
    if (!pScan->start(durationMs)) {
        scanningActive = false;
        DEBUG_println("startScan: failed to start BLE scan");
        return false;
    }
    return true;
}

bool BLE_YC01::init() {
    static bool initialized = false;
    if (initialized) return true;
    if (!NimBLEDevice::init("")) {
        Serial.println("FATAL: NimBLEDevice::init failed");
        return false;
    }
    Serial.print("BLE stack initialized. MAC: ");
    Serial.println(NimBLEDevice::getAddress().toString().c_str());
    initialized = true;
    return true;
}

bool BLE_YC01::isScanning() {
    return scanningActive;
}

std::vector<NimBLEAddress> BLE_YC01::getFoundDevices() {
    return foundDevices;
}

BLE_YC01::BLE_YC01(NimBLEAddress const& addr, String const& name) {
    this->address = addr;
    this->name = name;
    sensorType = "";
    memset(&readings, 0, sizeof(readings)); // Initialize readings
}

bool BLE_YC01::readData() {
    NimBLEClient *client = NimBLEDevice::createClient();
    if (!client) {
        DEBUG_println("readData: failed to create BLE client");
        return false;
    }

    client->setConnectTimeout(5); // fast fail on unreachable devices
    bool result = false;
    uint8_t retryCount = 0;
    do
    {
        esp_task_wdt_reset();
        result = false;

        if ( !client->connect(this->address) ) {
            DEBUG_printf("readData: connect failed (attempt %d)\n", retryCount + 1);
            retryCount++;
            continue;
        }

        this->sensorType = "";
        NimBLERemoteService* service;
        service = client->getService("1800");
        if (service) {
            NimBLERemoteCharacteristic* nameChar = service->getCharacteristic("2A00");
            if (nameChar && nameChar->canRead()) {
                this->sensorType = nameChar->readValue();
            }
        }

        service = client->getService(serviceUUID);
        if ( !service ) {
            DEBUG_println("readData: sensor service (ff01) not found");
            retryCount++;
            client->disconnect();
            continue;
        }

        NimBLERemoteCharacteristic *pCharacteristic = service->getCharacteristic(charUUID);
        if ( !pCharacteristic ) {
            DEBUG_println("readData: sensor characteristic (ff02) not found");
            retryCount++;
            client->disconnect();
            continue;
        }

        std::string value = pCharacteristic->readValue();
        int length = value.length();

        uint8_t decodedData[60];
        if ( !decodeData((uint8_t*)value.data(), length, decodedData) ) {
            DEBUG_println("readData: failed to decode data");
            retryCount++;
            client->disconnect();
            continue;
        }

        uint8_t chksum = checksum(decodedData, length-1);
        if (chksum != decodedData[length-1]) {
            DEBUG_println("readData: checksum mismatch");
            retryCount++;
            client->disconnect();
            continue;
        }

        struct sensorReadings_t readings;
        time_t now;
        time(&now);
        readings.time = now;
        readings.rssi = client->getRssi();
        readings.type = decodedData[2];
        readings.pH = toInt16(decodedData, 3) / 100.0;
        readings.ec = toInt16(decodedData, 5);
        readings.salt = toInt16(decodedData, 5) * 0.55;
        readings.tds = toInt16(decodedData, 7);
        readings.orp = toInt16(decodedData, 9);
        readings.cl = toInt16(decodedData, 11) / 10.0;
        readings.temp = toInt16(decodedData, 13) / 10.0;
        readings.bat = toInt16(decodedData, 15);
        this->readings = readings;

        result = true;

    } while ( !result && retryCount < 3 );

    NimBLEDevice::deleteClient(client);
    return result;
}
