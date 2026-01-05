#include "bme280.hpp"
#include "logger.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>


SysLogger logger("BME280");
//TODO use syslogger to debug this module
//Sets begining  string for all iio devices
BME280::BME280()
: device_path("/sys/bus/iio/devices/iio:device") {
}

BME280::~BME280() {}

// Must look at all iio devices and find one that has bme280 in this name file
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

float BME280::readFloat(const std::string& attr_name) {
    // std::ifstream file(device_path_ + "/" + attr_name);
    // if (!file.is_open()) {
    //     return 0.0f;
    // }
    
    // float value;
    // file >> value;
    // return value;
}

bool BME280::readSensorData(SensorData& data) {
    // data.temperature = readFloat("in_temp_input") / 1000.0f;  // millidegrees to Celsius
    // data.pressure = readFloat("in_pressure_input") / 10.0f;   // kPa to hPa
    // data.humidity = readFloat("in_humidityrelative_input") / 1000.0f;  // milli-percent to percent
    
    // return (data.temperature != 0.0f || data.pressure != 0.0f || data.humidity != 0.0f);
}

float BME280::readTemperature() {
    // return readFloat("in_temp_input") / 1000.0f;
}

float BME280::readPressure() {
    // return readFloat("in_pressure_input") / 10.0f;
}

float BME280::readHumidity() {
    // return readFloat("in_humidityrelative_input") / 1000.0f;
}
