#ifndef BME280_HPP
#define BME280_HPP

#include <cstdint>
#include <ctime>
#include <mutex>
#include <span>
#include <string>

#include "I2CBus.hpp"

struct BME280Data {
    float temperature;
    float pressure;
    float humidity;
    std::time_t timestamp;
};

class BME280 {
public:
    static constexpr const char* DEFAULT_BUS_PATH = "/dev/i2c-2";
    static constexpr uint8_t DEFAULT_ADDRESS = 0x76;  // 0x77 when SDO is tied high

    explicit BME280(const std::string& busPath = DEFAULT_BUS_PATH,
                    uint8_t address = DEFAULT_ADDRESS);
    ~BME280();

    BME280(const BME280&) = delete;
    BME280& operator=(const BME280&) = delete;

    int init();

    int read(BME280Data& data);

    int readTemperature(float& temp);
    int readPressure(float& press);
    int readHumidity(float& hum);

private:
    // Field names match the datasheet (section 4.2.2)
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

    int readRegisters(uint8_t reg, std::span<uint8_t> out);
    int writeRegister(uint8_t reg, uint8_t value);

    int checkChipId();
    int softReset();
    int readCalibration();
    int configure();

    int readRawData(int32_t& adcT, int32_t& adcP, int32_t& adcH);

    // Caller must hold m_mutex
    int readCompensated(float& temp, float& press, float& hum);

    // compensateTemperature must run first: it sets m_tFine, which the others use
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

#endif // BME280_HPP
