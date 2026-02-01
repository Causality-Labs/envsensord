#include <iostream>
#include <string>
#include "my_socket_lib.hpp"
#include "logger.hpp"
#include "bme280.hpp"
#include "SSNP.hpp"
#include "CommandLineParser.hpp"

int main(int argc, char* argv[])
{
    StdLogger logger("HW_Client");
    ssnp::SsnpClient SSNPParser;
    ClientCommandLineParser commandLineParser;
    EnvClientConfig config;
    BME280::SensorData dataPacket;
    int ret;

    ret = commandLineParser.parse(argc, argv, config);
    if (ret != 0) {
        commandLineParser.printUsage(argv[0]);
        return -1;
    }

    if (config.showHelp == true) {
        commandLineParser.printUsage(argv[0]);
        return 0;
    }
    
    if (config.showVersion == true) {
        commandLineParser.printVersion();
        return 0;
    }

    Client client(config.hostname);

    logger.info("Connecting to server at " + config.hostname + ":" + std::to_string(config.port));
    if (client.connect_to_server(config.port) < 0) {
        logger.error("Failed to connect to server");
        return -1;
    }
    
    logger.info("Connected to server!");

    std::string request_msg;
    ret = SSNPParser.buildRequest(config.request, request_msg);
    if (ret != 0) {
        logger.error("Failed to build request");
        client.disconnect();
        return -1;
    }

    if (client.send_data(request_msg) < 0) {
        logger.error("Failed to send data");
        client.disconnect();
        return -1;
    }

    std::string response;
    int bytes = client.receive_data(response);
    if (bytes <= 0) {
        logger.error("Failed to recieve data");
        client.disconnect();
        return -1;
    }


    logger.info("Recieved: " + response);

    ret = SSNPParser.parseResponse(response, dataPacket);
    if (ret != 0) {
        logger.error("Invalid response");
        client.disconnect();
        return -1;
    }

    logger.info("Timestamp: " + SSNPParser.getReadableTS(dataPacket));

    if (SSNPParser.has_temp() == true)
        logger.info("Temperature: " + std::to_string(dataPacket.temperature) + "°C");
    
    if (SSNPParser.has_press() == true)
        logger.info("Pressure: " + std::to_string(dataPacket.pressure) + " hPa");
    
    if (SSNPParser.has_humid() == true)
        logger.info("Humidity: " + std::to_string(dataPacket.humidity) + "%");

    client.disconnect();
    logger.info("Disconnected from server");

    return 0;
}
