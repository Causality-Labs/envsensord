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
    int readSensorData(SensorData& data);
    int readTemperature(float& temp);
    int readPressure(float& press);
    int readHumidity(float& hum);

private:
    std::string device_path;
    int readFloat(const std::string& attr_name, float& value);
};

#endif // BME280_HPP
