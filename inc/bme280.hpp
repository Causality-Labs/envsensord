#ifndef BME280_HPP
#define BME280_HPP

#include <string>

struct SensorData {
    float temperature;  // degrees Celsius
    float pressure;     // hectopascals (hPa)
    float humidity;     // % relative humidity
};

class BME280 {
public:
    BME280();
    ~BME280();

    int init();
    bool readSensorData(SensorData& data);
    float readTemperature();
    float readPressure();
    float readHumidity();

private:
    std::string device_path;
    float readFloat(const std::string& attr_name);
};

#endif // BME280_HPP
