#include <Arduino.h>
#include <NimBLEDevice.h>

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

static int16_t toInt16(uint8_t const data[], int idx) {
    return ((uint16_t)data[idx] << 8) | (uint16_t)data[idx + 1];
}

static uint8_t decodedData[60]; // Buffer for decoded data
static uint8_t* decodeData(uint8_t const data[], int length) {
    if (length < 2 || length > 60) {
        return NULL; // Invalid data length
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

        decodedData[i] = ~(hibit1|lobit0);
        tmp = ~(hibit0|lobit1);
        decodedData[i-1] = tmp;
    }
    return decodedData;
}

static uint8_t checksum(const uint8_t* data, int length)		
{		
    uint8_t i = 0;	
    uint8_t chksum = 0;	
    for(i = 0; i < length; i ++)	
        chksum = chksum ^ data[i];
    return chksum;	
}


std::vector<NimBLEAddress> BLE_YC01::scan()
{
    std::vector<NimBLEAddress> addrList;
    NimBLEDevice::init("");
    NimBLEScan *pScan = NimBLEDevice::getScan();
    NimBLEScanResults results = pScan->getResults(2500);

    // Iterate through the results
    for (int i = 0; i < results.getCount(); i++) {
        const NimBLEAdvertisedDevice *device = results.getDevice(i);
        if ( !device->isAdvertisingService(serviceUUID) ) {
            continue;
        }

        DEBUG_print("Found device: "); DEBUG_print(device->getName().c_str());
        DEBUG_print(" ("); DEBUG_print(device->getAddress().toString().c_str()); DEBUG_println(")");
    
        addrList.push_back(device->getAddress());
    }

    return addrList;
}

BLE_YC01::BLE_YC01(NimBLEAddress const& addr, String const& name) {
    this->address = addr;
    this->name = name;
    sensorType = "";
    memset(&readings, 0, sizeof(readings)); // Initialize readings
}

bool BLE_YC01::readData() {
    NimBLEClient *client = NimBLEDevice::createClient();
    if (!client) { // Make sure the client was created
        return false;
    }

    // connect to the device
    bool result = true;
    uint8_t retryCount = 0;
    do
    {
        if ( client->connect(this->address) ) {
            this->sensorType = "";
            NimBLERemoteService* service;
            service = client->getService("1800");
            if (service) {
                NimBLERemoteCharacteristic* nameChar = service->getCharacteristic("2A00");
                if (nameChar && nameChar->canRead()) {
                    this->sensorType = nameChar->readValue();
                }
            }

            // Check if the sensor service was found
            service = client->getService(serviceUUID);
            if ( service ) {
                NimBLERemoteCharacteristic *pCharacteristic = service->getCharacteristic(charUUID);
                if ( pCharacteristic ) {

                    // Read the raw value
                    std::string value = pCharacteristic->readValue();
                    int length = value.length();

                    // Decode the data
                    uint8_t *data = decodeData((uint8_t*)value.data(), length);
                    if ( data )
                    {
                        uint8_t chksum = checksum(data, length-1);
                        if (chksum != data[length-1]) {
                            DEBUG_println("Checksum mismatch!");
                            result = false;
                        } else {
                            struct sensorReadings_t readings;
                            time_t now;
                            time(&now);
                            readings.time = now; // Current time in seconds
                            readings.rssi = client->getRssi();
                            readings.type = data[2];
                            readings.pH = toInt16(data, 3) / 100.0; // pH value
                            readings.ec = toInt16(data, 5); // EC value in mV
                            readings.salt = toInt16(data, 5) * 0.55; // Salt value in g/L
                            readings.tds = toInt16(data, 7); // TDS value in mg/L
                            readings.orp = toInt16(data, 9); // ORP value in mV
                            readings.cl = toInt16(data, 11) / 10.0; // Chlorine value in mg/L
                            readings.temp = toInt16(data, 13) / 10.0; // Temperature value in °C
                            readings.bat = toInt16(data, 15); // Battery value in mV
                            this->readings = readings; // Store the readings
                        }
                    } else {
                        DEBUG_println("Failed to decode data");
                        result = false;
                    }
                }
            }
        } else {
            // failed to connect
            DEBUG_println("Failed to connect to device");
            result = false;
        }
    
        retryCount++;
    } while ( !result && retryCount < 3 );

    NimBLEDevice::deleteClient(client);            

    return result;
}
