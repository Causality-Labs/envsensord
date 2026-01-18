#ifndef SSNP_HPP
#define SSNP_SERVER_HPP


#include <string>
#include <cstdint>
#include "bme280.hpp"

namespace ssnp {

enum class SsnpType :
uint8_t
{
    REQUEST_TEMP = 0,
    REQUEST_PRESSURE = 1,
    REQUEST_HUMIDITY = 2,
    REQUEST_ALL = 3,
    RESPONSE_DATA = 4,
    ERROR = 5
};

struct SsnpRequestType {
    bool temp_req;
    bool press_req;
    bool hum_req;
    bool invalid_req;
};


class Ssnp {
public:
    Ssnp();
    ~Ssnp();
    SsnpRequestType req_type;

    // Server-side methods
    int parseRequest(const std::string& request);
    void buildResponse(const bme280::SensorData& data, std::string& response);

    // Client-side methods
    std::string buildRequest();
    bool parseResponse(const std::string& response, bme280::SensorData& data);

private:
    void construct_header(std::time_t timestamp, std::stringstream& header);
};

}  // namespace ssnp

#endif // Ssnp_SERVER_HPP
