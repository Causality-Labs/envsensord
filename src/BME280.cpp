#include "BME280.hpp"
#include "logger.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <thread>

namespace {

SysLogger syslogger("BME280");

// Register map (datasheet section 5.3)
constexpr uint8_t kRegCalib00   = 0x88;  // 0x88..0xA1: T and P calibration, then H1
constexpr uint8_t kRegChipId    = 0xD0;
constexpr uint8_t kRegReset     = 0xE0;
constexpr uint8_t kRegCalib26   = 0xE1;  // 0xE1..0xE7: H2..H6 calibration
constexpr uint8_t kRegCtrlHum   = 0xF2;
constexpr uint8_t kRegStatus    = 0xF3;
constexpr uint8_t kRegCtrlMeas  = 0xF4;
constexpr uint8_t kRegConfig    = 0xF5;
constexpr uint8_t kRegData      = 0xF7;  // 0xF7..0xFE: press, temp, hum raw data

constexpr uint8_t kChipId       = 0x60;
constexpr uint8_t kResetCommand = 0xB6;

constexpr size_t kCalib00Len    = 26;
constexpr size_t kCalib26Len    = 7;
constexpr size_t kDataLen       = 8;

// Status register: set while NVM calibration data is being copied to image registers
constexpr uint8_t kStatusImUpdate = 0x01;

// Settings (datasheet section 5.4).
// Oversampling x1 for all three channels, IIR filter off, normal mode with
// 125 ms standby. This gives a fresh sample roughly every 135 ms.
constexpr uint8_t kOsrsX1        = 0x01;
constexpr uint8_t kModeNormal    = 0x03;
constexpr uint8_t kStandby125ms  = 0x02;
constexpr uint8_t kFilterOff     = 0x00;

constexpr uint8_t kCtrlHumValue  = kOsrsX1;
constexpr uint8_t kCtrlMeasValue = (kOsrsX1 << 5) | (kOsrsX1 << 2) | kModeNormal;
constexpr uint8_t kConfigValue   = (kStandby125ms << 5) | (kFilterOff << 2);

// Operating range of the sensor (datasheet section 1), used to clamp results the
// same way Bosch's BME280_SensorAPI does
constexpr int32_t  kTempMin      = -4000;            // -40.00 degC
constexpr int32_t  kTempMax      = 8500;             //  85.00 degC
constexpr uint32_t kPressMin     = 30000u * 256u;    //  300 hPa in Q24.8 Pa
constexpr uint32_t kPressMax     = 110000u * 256u;   // 1100 hPa in Q24.8 Pa

// Soft reset: start-up time and how many times to poll for the NVM copy to finish
constexpr auto kStartupDelay     = std::chrono::milliseconds(2);
constexpr int kResetPollAttempts = 5;

// Raw values reported for a channel that has not been measured yet
constexpr int32_t kSkippedTP     = 0x80000;
constexpr int32_t kSkippedH      = 0x8000;

uint16_t toU16(const uint8_t* p)
{
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

int16_t toS16(const uint8_t* p)
{
    return static_cast<int16_t>(toU16(p));
}

}

BME280::BME280::BME280(const std::string& busPath, uint8_t address):
    m_bus(busPath),
    m_address(address)
{

}

BME280::BME280::~BME280()
{

}

int BME280::BME280::init()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_initialized = false;

    int ret = m_bus.init();
    if (ret < 0)
    {
        syslogger.error("Failed to open I2C bus: " + std::string(std::strerror(-ret)));
        return ret;
    }

    ret = checkChipId();
    if (ret < 0)
    {
        return ret;
    }

    ret = softReset();
    if (ret < 0)
    {
        syslogger.error("Soft reset failed: " + std::string(std::strerror(-ret)));
        return ret;
    }

    ret = readCalibration();
    if (ret < 0)
    {
        syslogger.error("Failed to read calibration: " + std::string(std::strerror(-ret)));
        return ret;
    }

    ret = configure();
    if (ret < 0)
    {
        syslogger.error("Failed to configure sensor: " + std::string(std::strerror(-ret)));
        return ret;
    }

    m_initialized = true;
    syslogger.info("BME280 initialized");
    return 0;
}

int BME280::BME280::read(SensorData& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return readCompensated(data.temperature, data.pressure, data.humidity);
}

int BME280::BME280::readTemperature(float& temp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    float press = 0.0f;
    float hum = 0.0f;
    return readCompensated(temp, press, hum);
}

int BME280::BME280::readPressure(float& press)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    float temp = 0.0f;
    float hum = 0.0f;
    return readCompensated(temp, press, hum);
}

int BME280::BME280::readHumidity(float& hum)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    float temp = 0.0f;
    float press = 0.0f;
    return readCompensated(temp, press, hum);
}

int BME280::BME280::readCompensated(float& temp, float& press, float& hum)
{
    if (!m_initialized)
    {
        return -ENODEV;
    }

    int32_t adcT = 0;
    int32_t adcP = 0;
    int32_t adcH = 0;

    int ret = readRawData(adcT, adcP, adcH);
    if (ret < 0)
    {
        syslogger.error("Failed to read measurement: " + std::string(std::strerror(-ret)));
        return ret;
    }

    // Temperature first: it updates m_tFine, which pressure and humidity depend on
    int32_t t = compensateTemperature(adcT);
    uint32_t p = compensatePressure(adcP);
    uint32_t h = compensateHumidity(adcH);

    if (p == 0)
    {
        return -EIO;  // invalid calibration (division by zero guard)
    }

    temp  = static_cast<float>(t) / 100.0f;
    press = static_cast<float>(p) / 25600.0f;  // Q24.8 Pa -> hPa
    hum   = static_cast<float>(h) / 1024.0f;   // Q22.10 %RH
    return 0;
}

int BME280::BME280::readRegisters(uint8_t reg, std::span<uint8_t> out)
{
    I2CBus::i2c_msg_t messages[] =
    {
        {m_address, false, {&reg, 1}},
        {m_address, true,  out},
    };

    return m_bus.transfer(messages);
}

int BME280::BME280::writeRegister(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};

    I2CBus::i2c_msg_t messages[] =
    {
        {m_address, false, buf},
    };

    return m_bus.transfer(messages);
}

int BME280::BME280::checkChipId()
{
    uint8_t id = 0;

    int ret = readRegisters(kRegChipId, {&id, 1});
    if (ret < 0)
    {
        syslogger.error("Failed to read chip ID: " + std::string(std::strerror(-ret)));
        return ret;
    }

    if (id != kChipId)
    {
        syslogger.error("Unexpected chip ID " + std::to_string(id) + ", not a BME280");
        return -ENODEV;
    }

    return 0;
}

int BME280::BME280::softReset()
{
    int ret = writeRegister(kRegReset, kResetCommand);
    if (ret < 0)
    {
        return ret;
    }

    // Wait for start-up, then for the calibration data to be copied from NVM
    for (int attempt = 0; attempt < kResetPollAttempts; ++attempt)
    {
        std::this_thread::sleep_for(kStartupDelay);

        uint8_t status = 0;
        ret = readRegisters(kRegStatus, {&status, 1});
        if (ret < 0)
        {
            return ret;
        }

        if ((status & kStatusImUpdate) == 0)
        {
            return 0;
        }
    }

    return -ETIMEDOUT;
}

int BME280::BME280::readCalibration()
{
    uint8_t c0[kCalib00Len] = {};
    uint8_t c1[kCalib26Len] = {};

    int ret = readRegisters(kRegCalib00, c0);
    if (ret < 0)
    {
        return ret;
    }

    ret = readRegisters(kRegCalib26, c1);
    if (ret < 0)
    {
        return ret;
    }

    // 0x88..0x9F: little-endian 16-bit values
    m_calib.digT1 = toU16(&c0[0]);
    m_calib.digT2 = toS16(&c0[2]);
    m_calib.digT3 = toS16(&c0[4]);
    m_calib.digP1 = toU16(&c0[6]);
    m_calib.digP2 = toS16(&c0[8]);
    m_calib.digP3 = toS16(&c0[10]);
    m_calib.digP4 = toS16(&c0[12]);
    m_calib.digP5 = toS16(&c0[14]);
    m_calib.digP6 = toS16(&c0[16]);
    m_calib.digP7 = toS16(&c0[18]);
    m_calib.digP8 = toS16(&c0[20]);
    m_calib.digP9 = toS16(&c0[22]);
    // c0[24] (0xA0) is unused
    m_calib.digH1 = c0[25];  // 0xA1

    // 0xE1..0xE7
    m_calib.digH2 = toS16(&c1[0]);
    m_calib.digH3 = c1[2];

    // H4 and H5 are signed 12-bit values that share register 0xE5:
    //   H4 = 0xE4[7:0] << 4 | 0xE5[3:0]
    //   H5 = 0xE6[7:0] << 4 | 0xE5[7:4]
    m_calib.digH4 = static_cast<int16_t>(static_cast<int8_t>(c1[3]) * 16 | (c1[4] & 0x0F));
    m_calib.digH5 = static_cast<int16_t>(static_cast<int8_t>(c1[5]) * 16 | (c1[4] >> 4));
    m_calib.digH6 = static_cast<int8_t>(c1[6]);

    return 0;
}

int BME280::BME280::configure()
{
    // The sensor is in sleep mode after reset, so config can be written safely.
    // ctrl_hum only takes effect after the following write to ctrl_meas.
    int ret = writeRegister(kRegCtrlHum, kCtrlHumValue);
    if (ret < 0)
    {
        return ret;
    }

    ret = writeRegister(kRegConfig, kConfigValue);
    if (ret < 0)
    {
        return ret;
    }

    ret = writeRegister(kRegCtrlMeas, kCtrlMeasValue);
    if (ret < 0)
    {
        return ret;
    }

    // Wait for the first measurement to complete so the first read is valid
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return 0;
}

int BME280::BME280::readRawData(int32_t& adcT, int32_t& adcP, int32_t& adcH)
{
    // Burst read all data registers so the three values come from the same measurement
    uint8_t d[kDataLen] = {};

    int ret = readRegisters(kRegData, d);
    if (ret < 0)
    {
        return ret;
    }

    adcP = (static_cast<int32_t>(d[0]) << 12) | (static_cast<int32_t>(d[1]) << 4) | (d[2] >> 4);
    adcT = (static_cast<int32_t>(d[3]) << 12) | (static_cast<int32_t>(d[4]) << 4) | (d[5] >> 4);
    adcH = (static_cast<int32_t>(d[6]) << 8)  |  static_cast<int32_t>(d[7]);

    if (adcT == kSkippedTP || adcP == kSkippedTP || adcH == kSkippedH)
    {
        return -EAGAIN;  // no measurement available yet
    }

    return 0;
}

// The three compensation functions below are based on the integer reference code in
// the Bosch BME280 datasheet (BST-BME280-DS002, section 4.2.3). C++20 defines shifts
// of negative values, so the arithmetic matches the datasheet exactly. As in Bosch's
// BME280_SensorAPI, results are clamped to the sensor's operating range.

int32_t BME280::BME280::compensateTemperature(int32_t adcT)
{
    const int32_t t1 = m_calib.digT1;
    const int32_t t2 = m_calib.digT2;
    const int32_t t3 = m_calib.digT3;

    int32_t var1 = (((adcT >> 3) - (t1 << 1)) * t2) >> 11;
    int32_t var2 = (((((adcT >> 4) - t1) * ((adcT >> 4) - t1)) >> 12) * t3) >> 14;

    m_tFine = var1 + var2;

    int32_t temp = (m_tFine * 5 + 128) >> 8;
    return std::clamp(temp, kTempMin, kTempMax);
}

uint32_t BME280::BME280::compensatePressure(int32_t adcP)
{
    int64_t var1 = static_cast<int64_t>(m_tFine) - 128000;
    int64_t var2 = var1 * var1 * m_calib.digP6;
    var2 = var2 + ((var1 * m_calib.digP5) << 17);
    var2 = var2 + (static_cast<int64_t>(m_calib.digP4) << 35);
    var1 = ((var1 * var1 * m_calib.digP3) >> 8) + ((var1 * m_calib.digP2) << 12);
    var1 = (((static_cast<int64_t>(1) << 47) + var1) * m_calib.digP1) >> 33;

    if (var1 == 0)
    {
        return 0;  // avoid division by zero
    }

    int64_t p = 1048576 - adcP;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (static_cast<int64_t>(m_calib.digP9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (static_cast<int64_t>(m_calib.digP8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (static_cast<int64_t>(m_calib.digP7) << 4);

    return static_cast<uint32_t>(std::clamp<int64_t>(p, kPressMin, kPressMax));
}

uint32_t BME280::BME280::compensateHumidity(int32_t adcH)
{
    const int32_t h1 = m_calib.digH1;
    const int32_t h2 = m_calib.digH2;
    const int32_t h3 = m_calib.digH3;
    const int32_t h4 = m_calib.digH4;
    const int32_t h5 = m_calib.digH5;
    const int32_t h6 = m_calib.digH6;

    int32_t v = m_tFine - 76800;

    v = (((((adcH << 14) - (h4 << 20) - (h5 * v)) + 16384) >> 15) *
         (((((((v * h6) >> 10) * (((v * h3) >> 11) + 32768)) >> 10) + 2097152) * h2 + 8192) >> 14));
    v = v - (((((v >> 15) * (v >> 15)) >> 7) * h1) >> 4);
    v = (v < 0) ? 0 : v;
    v = (v > 419430400) ? 419430400 : v;

    return static_cast<uint32_t>(v >> 12);
}
