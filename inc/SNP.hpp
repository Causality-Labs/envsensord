#ifndef SNP_HPP
#define SNP_SERVER_HPP


#include <string>
#include <cstdint>
#include "bme280.hpp"

enum class SnpType :
uint8_t
{
    REQUEST_TEMP = 0,
    REQUEST_PRESSURE = 1,
    REQUEST_HUMIDITY = 2,
    REQUEST_ALL = 3,
    RESPONSE_DATA = 4,
    ERROR = 5
};


class SNP {
public:
    SNP();
    ~SNP();

    // Server-side methods
    SnpType parseRequest(const std::string& request);
    std::string buildResponse(const SensorData& data, SnpType requestType);
    std::string buildErrorResponse(const std::string& errorMsg);

    // Client-side methods
    std::string buildRequest(SnpType requestType);
    bool parseResponse(const std::string& response, SensorData& data);

private:
};


#endif // SNP_SERVER_HPP
