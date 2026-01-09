#include "SNP.hpp"
#include "bme280.hpp"  // For SensorData struct
#include "logger.hpp"
#include <sstream>
#include <iomanip>

SysLogger protocol_logger("SnpProtocol");

SNP::SNP() {}

SNP::~SNP() {}

SnpType SNP::parseRequest(const std::string& data)
{
    if (data.empty())
        return SnpType::ERROR;
    
    // Simple text-based protocol parsing
    if (data.find("TEMP") != std::string::npos) {
        return SnpType::REQUEST_TEMP;
    }
    else if (data.find("PRESSURE") != std::string::npos) {
        return SnpType::REQUEST_PRESSURE;
    }
    else if (data.find("HUMIDITY") != std::string::npos) {
        return SnpType::REQUEST_HUMIDITY;
    } 
    else if (data.find("ALL") != std::string::npos) {
        return SnpType::REQUEST_ALL;
    }
    
    return SnpType::ERROR;
}

std::string SNP::buildResponse(const SensorData& data, SnpType requestType)
{
    std::ostringstream response;
    response << std::fixed << std::setprecision(2);
    
    switch (requestType) {
        case SnpType::REQUEST_TEMP:
            response << "TEMP:" << data.temperature << "C";
            break;
            
        case SnpType::REQUEST_PRESSURE:
            response << "PRESSURE:" << data.pressure << "hPa";
            break;
            
        case SnpType::REQUEST_HUMIDITY:
            response << "HUMIDITY:" << data.humidity << "%";
            break;
            
        case SnpType::REQUEST_ALL:
            response << "TEMP:" << data.temperature << "C,"
                    << "PRESSURE:" << data.pressure << "hPa,"
                    << "HUMIDITY:" << data.humidity << "%";
            break;
            
        default:
            return buildErrorResponse("Invalid request type");
    }
    
    return response.str();
}

std::string SNP::buildErrorResponse(const std::string& errorMsg)
{
    return "ERROR:" + errorMsg;
}

std::string SNP::buildRequest(SnpType requestType)
{
    switch (requestType) {
        case SnpType::REQUEST_TEMP:
            return "TEMP";
            
        case SnpType::REQUEST_PRESSURE:
            return "PRESSURE";
            
        case SnpType::REQUEST_HUMIDITY:
            return "HUMIDITY";
            
        case SnpType::REQUEST_ALL:
            return "ALL";
            
        default:
            protocol_logger.error("Invalid request type for buildRequest");
            return "ERROR";
    }
}

bool SNP::parseResponse(const std::string& response, SensorData& data)
{
    if (response.empty()) {
        protocol_logger.error("Empty response");
        return false;
    }
    
    // Check for error response
    if (response.find("ERROR:") != std::string::npos) {
        protocol_logger.error("Server returned error: " + response);
        return false;
    }
    
    // Initialize data
    data.temperature = 0.0f;
    data.pressure = 0.0f;
    data.humidity = 0.0f;
    
    // Parse response - format examples:
    // "TEMP:23.45C"
    // "PRESSURE:1013.25hPa"
    // "HUMIDITY:65.00%"
    // "TEMP:23.45C,PRESSURE:1013.25hPa,HUMIDITY:65.00%"
    
    size_t pos = 0;
    std::string resp = response;
    
    // Parse TEMP
    pos = resp.find("TEMP:");
    if (pos != std::string::npos) {
        size_t start = pos + 5;  // Skip "TEMP:"
        size_t end = resp.find("C", start);
        if (end != std::string::npos) {
            std::string temp_str = resp.substr(start, end - start);
            data.temperature = std::stof(temp_str);
        }
    }
    
    // Parse PRESSURE
    pos = resp.find("PRESSURE:");
    if (pos != std::string::npos) {
        size_t start = pos + 9;  // Skip "PRESSURE:"
        size_t end = resp.find("hPa", start);
        if (end != std::string::npos) {
            std::string press_str = resp.substr(start, end - start);
            data.pressure = std::stof(press_str);
        }
    }
    
    // Parse HUMIDITY
    pos = resp.find("HUMIDITY:");
    if (pos != std::string::npos) {
        size_t start = pos + 9;  // Skip "HUMIDITY:"
        size_t end = resp.find("%", start);
        if (end != std::string::npos) {
            std::string hum_str = resp.substr(start, end - start);
            data.humidity = std::stof(hum_str);
        }
    }
    
    return true;
}
