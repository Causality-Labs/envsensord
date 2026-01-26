#include "SSNP.hpp"
#include "bme280.hpp"  // For SensorData struct
#include "logger.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace ssnp;
SysLogger protocol_logger("SsnpProtocol");

SsnpServer::SsnpServer()
: req_type{false, false, false, false}
{}

SsnpServer::~SsnpServer() {}

void SsnpServer::construct_header(std::time_t timestamp, std::stringstream& header)
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
int SsnpServer::parseRequest(const std::string& request)
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

void SsnpServer::buildResponse(const BME280::SensorData& data, std::string& response)
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

SsnpClient::SsnpClient()
: outgoing_req{false, false, false, false}, incoming_res{false, false, false, false}
{}

SsnpClient::~SsnpClient() {}

int SsnpClient::buildRequest(const SsnpRequestType& request, std::string& request_str)
{
    if (request.temp_req == false &&
        request.press_req == false &&
        request.hum_req == false) {
        return -1;
    }

    // Store the outgoing request for later validation
    outgoing_req = request;

    std::stringstream ss;
    bool first = true;

    if (request.temp_req) {
        ss << "TEMP";
        first = false;
    }

    if (request.press_req) {
        if (!first) ss << ",";
        ss << "PRESS";
        first = false;
    }

    if (request.hum_req) {
        if (!first) ss << ",";
        ss << "HUMID";
    }

    ss << ";";
    request_str = ss.str();

    return 0;
}

void SsnpClient::parse_header(const std::string& response)
{
    // Find semicolon that separates header from data
    size_t semicolon_pos = response.find(';');
    if (semicolon_pos == std::string::npos) {
        incoming_res.invalid_req = true;
        return;
    }

    // Parse header to determine which fields are present
    header = response.substr(0, semicolon_pos);
    payload = response.substr(semicolon_pos + 1);

    // Determine field order from header
    incoming_res.temp_req = (header.find("TEMP") != std::string::npos);
    incoming_res.press_req = (header.find("PRESS") != std::string::npos);
    incoming_res.hum_req = (header.find("HUMID") != std::string::npos);

    return;
}

bool SsnpClient::check_response()
{
    if (incoming_res.hum_req != outgoing_req.hum_req)
        return false;
    
    if (incoming_res.press_req != outgoing_req.press_req)
        return false;

    if (incoming_res.temp_req != outgoing_req.temp_req)
        return false;

    return true;
}


int SsnpClient::parseResponse(const std::string& response, BME280::SensorData& data)
{
    if (response.empty() == true) {
        std::cout << "Payload empty";
        return -1;
    }

    parse_header(response);

    if (incoming_res.invalid_req == true) {
        std::cout << "Payload invalid";
        return -1;
    }

    if (check_response() == false) {
        std::cout << "Response  does not match";
        return -1;
    }



    // Parse data part: timestamp,value1,value2,...
    std::stringstream ss(payload);
    std::string token;

    // Parse timestamp
    if (!std::getline(ss, token, ',')) {
        return -1;
    }
    data.timestamp = std::stol(token);

    // Parse values in order based on what's in header
    if (incoming_res.temp_req == true) {
        if (!std::getline(ss, token, ',')) {
            return -1;
        }
        // Remove 'C' suffix and convert
        data.temperature = std::stof(token.substr(0, token.length() - 1));
    }

    if (incoming_res.press_req == true) {
        if (!std::getline(ss, token, ',')) {
            return -1;
        }
        // Remove 'hPa' suffix and convert
        data.pressure = std::stof(token.substr(0, token.length() - 3));
    }

    if (incoming_res.hum_req == true) {
        if (!std::getline(ss, token, ',')) {
            return -1;
        }
        // Remove '%' suffix and convert
        data.humidity = std::stof(token.substr(0, token.length() - 1));
    }

    return 0;
}

bool SsnpClient::has_temp()
{
    return incoming_res.temp_req;
}

bool SsnpClient::has_press()
{
    return incoming_res.press_req;
}

bool SsnpClient::has_humid()
{
    return incoming_res.hum_req;
}

std::string SsnpClient::getReadableTS(BME280::SensorData& data)
{
    std::stringstream ss;
    std::time_t timestamp = data.timestamp;
    std::tm* tm_info = std::localtime(&timestamp);
    
    ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}