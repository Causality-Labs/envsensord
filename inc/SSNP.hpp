#ifndef SSNP_HPP
#define SSNP_HPP


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


class SsnpServer {
public:
    SsnpServer();
    ~SsnpServer();
    SsnpRequestType req_type;

    // Server-side methods
    int parseRequest(const std::string& request);
    void buildResponse(const BME280::SensorData& data, std::string& response);

private:
    void construct_header(std::time_t timestamp, std::stringstream& header);
};

class SsnpClient {
public:
    SsnpClient();
    ~SsnpClient();

    // Client-side methods
    int buildRequest(const SsnpRequestType& request, std::string& request_str);
    int parseResponse(const std::string& response, BME280::SensorData& data);
    bool has_temp();
    bool has_press();
    bool has_humid();
    std::string getReadableTS(BME280::SensorData& data);

private:
    SsnpRequestType outgoing_req;
    SsnpRequestType incoming_res;
    std::string header;
    std::string payload;

    void parse_header(const std::string& response);
    bool check_response();
};

}  // namespace ssnp

#endif // SSNP_HPP
