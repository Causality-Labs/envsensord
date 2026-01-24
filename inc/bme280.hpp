#ifndef BME280_HPP
#define BME280_HPP

#include <string>
#include <cstdint>
#include <chrono>
#include <ctime>
#include <mutex>

namespace BME280 {

struct SensorData {
    float temperature;  // degrees Celsius
    float pressure;     // hectopascals (hPa)
    float humidity;     // % relative humidity
    std::time_t timestamp; // milliseconds since Unix epoch
};

class BME280 {
public:
    BME280();
    ~BME280();

    int init();
    int init(const std::string& device_path);
    int readTemperature(float& temp);
    int readPressure(float& press);
    int readHumidity(float& hum);

private:
    std::string device_path;
    std::mutex sensorMutex; 
    int readFloat(const std::string& attr_name, float& value);
};

}

#endif // BME280_HPP
