#include "SSNP.hpp"
#include "bme280.hpp"  // For SensorData struct
#include "logger.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace ssnp;
SysLogger protocol_logger("SsnpProtocol");

Ssnp::Ssnp()
: req_type{false, false, false, false}
{}

Ssnp::~Ssnp() {}

void Ssnp::construct_header(std::time_t timestamp, std::stringstream& header)
{
    bool first = true;
    
    if (req_type.temp_req) {
        header << "TEMP";
        first = false;
    }
    
    if (req_type.press_req) {
        if (!first) header << ",";
        header << "PRESS";
        first = false;
    }
    
    if (req_type.hum_req) {
        if (!first) header << ",";
        header << "HUMID";
    }
    
    header << ";" << timestamp;
    return;
}

// Client Request: "TEMP,PRESS,HUMID;"
int Ssnp::parseRequest(const std::string& request)
{
    // Reset request flags for new request
    req_type.temp_req = false;
    req_type.press_req = false;
    req_type.hum_req = false;
    req_type.invalid_req = false;

    if (request.empty() == true || request.back() != ';') {
        req_type.invalid_req = true;
        return -1;
    }

    std::stringstream ss(request.substr(0, request.length() - 1));
    std::string token;
    int field_count = 0;

    while (std::getline(ss, token, ','))
    {
        field_count++;

        // Check if more than 3 fields
        if (field_count > 3) {
            req_type.invalid_req = true;
            return -1;
        }

        if (token == "TEMP") {
            if (req_type.temp_req) {  // Already set, duplicate detected
                req_type.invalid_req = true;
                return -1;
            }
            req_type.temp_req = true;
        }
        else if (token == "PRESS") {
            if (req_type.press_req) {  // Already set, duplicate detected
                req_type.invalid_req = true;
                return -1;
            }
            req_type.press_req = true;
        }
        else if (token == "HUMID") {
            if (req_type.hum_req) {  // Already set, duplicate detected
                req_type.invalid_req = true;
                return -1;
            }
            req_type.hum_req = true;
        }
        else {
            // Unknown token
            req_type.invalid_req = true;
            return -1;
        }
    }
    
    // Check if at least one valid request was made
    if (!req_type.temp_req && !req_type.press_req && !req_type.hum_req) {
        req_type.invalid_req = true;
    }

    return 0;
}

void Ssnp::buildResponse(const bme280::SensorData& data, std::string& response)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    // Build header with requested fields and timestamp
    construct_header(data.timestamp, ss);

    // Add sensor data values in same order
    if (req_type.temp_req)
        ss << "," << data.temperature << "C";

    if (req_type.press_req)
        ss << "," << data.pressure << "hPa";

    if (req_type.hum_req)
        ss << "," << data.humidity << "%";

    response = ss.str();
    return;
}


std::string Ssnp::buildRequest()
{

}

bool Ssnp::parseResponse(const std::string& response, bme280::SensorData& data)
{

}
