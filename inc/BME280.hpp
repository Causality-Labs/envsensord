#ifndef BME280_HPP
#define BME280_HPP

#include <cstdint>
#include <ctime>
#include <mutex>
#include <span>
#include <string>

#include "I2CBus.hpp"

namespace BME280 {

struct SensorData {
    float temperature;  // degrees Celsius
    float pressure;     // hectopascals (hPa)
    float humidity;     // % relative humidity
    std::time_t timestamp; // milliseconds since Unix epoch
};

// Userspace driver for the Bosch BME280 over /dev/i2c-N.
// All public functions return 0 on success or -errno on failure.
class BME280 {
public:
    static constexpr const char* kDefaultBusPath = "/dev/i2c-0";
    static constexpr uint8_t kDefaultAddress = 0x76;  // 0x77 when SDO is tied high

    explicit BME280(const std::string& busPath = kDefaultBusPath,
                    uint8_t address = kDefaultAddress);
    ~BME280();

    // Opens the bus, verifies the chip, loads calibration and starts measuring
    int init();

    // Reads temperature, pressure and humidity from a single measurement.
    // Does not set data.timestamp.
    int read(SensorData& data);

    int readTemperature(float& temp);
    int readPressure(float& press);
    int readHumidity(float& hum);

private:
    // Factory trimming values read from the sensor's NVM (datasheet section 4.2.2)
    struct Calibration
    {
        uint16_t digT1;
        int16_t  digT2;
        int16_t  digT3;

        uint16_t digP1;
        int16_t  digP2;
        int16_t  digP3;
        int16_t  digP4;
        int16_t  digP5;
        int16_t  digP6;
        int16_t  digP7;
        int16_t  digP8;
        int16_t  digP9;

        uint8_t  digH1;
        int16_t  digH2;
        uint8_t  digH3;
        int16_t  digH4;
        int16_t  digH5;
        int8_t   digH6;
    };

    // Register access built on I2CBus::transfer
    int readRegisters(uint8_t reg, std::span<uint8_t> out);
    int writeRegister(uint8_t reg, uint8_t value);

    // Setup steps called from init()
    int checkChipId();
    int softReset();
    int readCalibration();
    int configure();

    // Reads the raw 20-bit temperature/pressure and 16-bit humidity ADC values
    int readRawData(int32_t& adcT, int32_t& adcP, int32_t& adcH);

    // Reads one measurement and converts it. Caller must hold m_mutex.
    int readCompensated(float& temp, float& press, float& hum);

    // Compensation formulas (datasheet section 4.2.3).
    // compensateTemperature must run first, as it sets m_tFine for the others.
    int32_t  compensateTemperature(int32_t adcT);   // 0.01 degC
    uint32_t compensatePressure(int32_t adcP);      // Pa in Q24.8
    uint32_t compensateHumidity(int32_t adcH);      // %RH in Q22.10

    I2CBus m_bus;
    uint8_t m_address;
    bool m_initialized = false;
    Calibration m_calib = {};
    int32_t m_tFine = 0;
    std::mutex m_mutex;
};

}

#endif // BME280_HPP
