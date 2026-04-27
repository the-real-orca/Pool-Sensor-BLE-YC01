#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

/**
 * @brief Structure to hold sensor readings from BLE-YC01
 */
struct sensorReadings_t
{
    uint8_t type;       /**< Sensor type identifier */
    time_t time;        /**< Timestamp of the reading */
    int16_t rssi;       /**< BLE signal strength (RSSI) */
    float pH;           /**< pH value (0-14) */
    float ec;           /**< Electrical Conductivity in mV */
    float salt;         /**< Salinity in g/L */
    float tds;          /**< Total Dissolved Solids in mg/L */
    float orp;          /**< Oxidation-Reduction Potential in mV */
    float cl;           /**< Residual Chlorine in mg/L */
    float temp;         /**< Temperature in °C */
    float bat;          /**< Battery voltage in mV */
};

/**
 * @brief Compares a NimBLEAddress with a string representation
 * @param address The NimBLEAddress to compare
 * @param targetAddress The string representation (e.g., "AA:BB:CC:DD:EE:FF")
 * @return true if addresses match, false otherwise
 */
bool compareBLEAddress(const NimBLEAddress& address, const char* targetAddress);

/**
 * @brief Compares a NimBLEAddress with a String representation
 * @param address The NimBLEAddress to compare
 * @param targetAddress The String representation
 * @return true if addresses match, false otherwise
 */
bool compareBLEAddress(const NimBLEAddress& address, const String& targetAddress);
/**
 * @brief Class to handle communication with BLE-YC01 sensor
 */
class BLE_YC01 {
public:
    /**
     * @brief Starts an asynchronous scan for available BLE-YC01 devices.
     * @param duration Scan duration in seconds.
     * @return true if scan started successfully.
     */
    static bool startScan(uint32_t duration = 3);

    /**
     * @brief Checks if a scan is currently running.
     * @return true if scanning.
     */
    static bool isScanning();

    /**
     * @brief Gets the addresses of found devices after a scan is complete.
     * @return A vector of NimBLEAddress.
     */
    static std::vector<NimBLEAddress> getFoundDevices();

    /**
     * @brief Constructor for BLE_YC01
...
     * @param addr The BLE address of the sensor
     * @param name Optional name for the sensor
     */
    BLE_YC01(NimBLEAddress const& addr, String const& name = "");

    /**
     * @brief Gets the BLE address of the sensor
     * @return NimBLEAddress
     */
    NimBLEAddress getAddress() const { return address; }

    /**
     * @brief Gets the name of the sensor
     * @return String name
     */
    String getName() const { return name; }

    /**
     * @brief Sets the name of the sensor
     * @param name The new name
     */
    void setName(String const& name) { this->name = name; }

    /**
     * @brief Gets the sensor model type string
     * @return String sensor type
     */
    String getSensorType() const { return sensorType; }

    /**
     * @brief Gets the latest sensor readings
     * @return sensorReadings_t structure
     */
    sensorReadings_t getReadings() const { return readings; }

    /**
     * @brief Connects to the sensor and reads the current data
     * @return true if data was read successfully, false otherwise
     */
    bool readData();

protected:
    NimBLEAddress address; /**< Address of the BLE device */
    String sensorType;     /**< Model name/type of the sensor */
    String name;           /**< Custom name for the sensor */
    sensorReadings_t readings; /**< Last read sensor data */
};
