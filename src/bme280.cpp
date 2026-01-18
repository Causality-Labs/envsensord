#include "bme280.hpp"
#include "logger.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using namespace bme280;

SysLogger logger("BME280");

BME280::BME280()
: device_path("/sys/bus/iio/devices/iio:device") {
}

BME280::~BME280() {}

int BME280::init()
{
    for (int i = 0; i < 10; i++)
    {
        std::string test_path = device_path + std::to_string(i);  // Declare test_path
        std::string name_file = test_path + "/name";

        std::ifstream name_stream(name_file);
        if (name_stream.is_open() == false)
            continue;

        std::string device_name;
        std::getline(name_stream, device_name);
        name_stream.close();
        
        if (device_name.find("bme280") == std::string::npos)
            continue;

        device_path = test_path;

        logger.error("Found device");
        return 0;
    }

    logger.error("Could not find device");
    return -1;
}

int BME280::readFloat(const std::string& attr_name, float& value)
{
    std::string file_path = device_path + "/" + attr_name;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        logger.error("Failed to open: " + file_path);
        return -1;
    }
    
    file >> value;
    logger.info("Read " + attr_name + ": " + std::to_string(value));
    return 0;
}

int BME280::readSensorData(SensorData& data)
{
    float temp_raw;
    float press_raw;
    float hum_raw;

    if (readFloat("in_temp_input", temp_raw) != 0) {
        logger.error("Failed to read temperature");
        return -1;
    }

    if (readFloat("in_pressure_input", press_raw) != 0) {
        logger.error("Failed to read pressure");
        return -1;
    }

    if (readFloat("in_humidityrelative_input", hum_raw) != 0) {
        logger.error("Failed to read humidity");
        return -1;
    }

    data.temperature = temp_raw / 1000.0f;  // millidegrees to Celsius
    data.pressure = press_raw / 10.0f;       // kPa to hPa
    data.humidity = hum_raw / 1000.0f;       // milli-percent to percent

    return 0;
}

int BME280::readTemperature(float& temp)
{
    float temp_raw;
    if (readFloat("in_temp_input", temp_raw) != 0) {
        logger.error("Failed to read temperature");
        return -1;
    }

    temp = temp_raw / 1000.0f;
    return 0;
}

int BME280::readPressure(float& press)
{
    float press_raw;
    if (readFloat("in_pressure_input", press_raw) != 0) {
        logger.error("Failed to read pressure");
        return -1;
    }

    press = press_raw / 10.0f;
    return 0;
}

int BME280::readHumidity(float& hum)
{
    float hum_raw;
    if (readFloat("in_humidityrelative_input", hum_raw) != 0) {
        logger.error("Failed to read humidity");
        return -1;
    }

    hum = hum_raw / 1000.0f;
    return 0;
}
