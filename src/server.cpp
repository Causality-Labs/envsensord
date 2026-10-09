#include <iostream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <thread>
#include "my_socket_lib.hpp"
#include "logger.hpp"
#include "BME280.hpp"
#include "SSNP.hpp"
#include "ThreadPool.hpp"
#include "SensorManager.hpp"
#include "CommandLineParser.hpp"

using BME280SensorManager = SensorManager<BME280::BME280, BME280::SensorData>;

StdLogger stdlogger("HW_Server");
Server server;

void handleClientCallback(int client_fd, BME280SensorManager& sensorMgr);
void readBME280DataCallback(BME280::BME280& sensor, BME280::SensorData& data);

int main(int argc, char* argv[])
{
    ServerCommandLineParser parser;
    EnvServerConfig config;
    int ret{};

    ret = parser.parse(argc, argv, config);
    if (ret != 0) {
        parser.printUsage(argv[0]);
        return -1;
    }

    if (config.showHelp == true) {
        parser.printUsage(argv[0]);
        return 0;
    }
    
    if (config.showVersion == true) {
        parser.printVersion();
        return 0;
    }

    const std::string busPath = config.deviceName.empty() ? BME280::BME280::kDefaultBusPath
                                                          : config.deviceName;
    BME280::BME280 bme280(busPath);

    ret = bme280.init();
    if (ret != 0) {
        stdlogger.error("Was not able to initialize bme280 sensor on " + busPath + ".");
        return -1;
    }

    BME280SensorManager sensorMgr(bme280, readBME280DataCallback);
    sensorMgr.setInterval(config.sensorInterval);

    stdlogger.info("Initialized bme280 sensor.");

    ret = server.connect_to_port(config.port, 25);
    if (ret < 0) {
        stdlogger.error("Failed to start server");
        return 1;
    }

    stdlogger.info("Server listening on port 3500...");

    ThreadPool<int> thread_pool(config.numThreads, [&sensorMgr](int client_fd) {
        handleClientCallback(client_fd, sensorMgr);
    });
    sensorMgr.start();

    while (1)
    {
        int client_fd = server.wait_for_connection();
        if (client_fd <= 0) {
            stdlogger.error("Failed to connect to a client");
            continue;
        }

        stdlogger.info("Client connected");

        thread_pool.enqueue(client_fd); 
    }
    return 0;
}

void handleClientCallback(int client_fd, BME280SensorManager& sensorMgr)
{

    ssnp::SsnpServer SSNPParser;
    int ret;

    // Recieve task data (client_fd,)
    std::string client_request{};
    ret = server.receive_request(client_fd, client_request);

    if (ret < 0) {
        stdlogger.error("Failed to recieve data from client.");
        close(client_fd);
        return;
    }

    stdlogger.info("Client Request: " + client_request);

    // Parse data recieved
    ret = SSNPParser.parseRequest(client_request);
    if (ret < 0 || SSNPParser.req_type.invalid_req == true) {
        stdlogger.error("Invalid Client Request");
        close(client_fd);
        return;
    }

    BME280::SensorData sensor_data;
    sensorMgr.getData(sensor_data); 

    std::string client_response{};
    SSNPParser.buildResponse(sensor_data, client_response);

    ret = server.send_data(client_fd, client_response);
    if (ret == -1) {
        stdlogger.error("Failed to send data to server");
        close(client_fd);
        return;
    }

    stdlogger.info("Response sent to client");
    close(client_fd);
}

void readBME280DataCallback(BME280::BME280& sensor, BME280::SensorData& data)
{
    int ret = sensor.read(data);
    if (ret != 0) {
        stdlogger.error("Failed to read bme280 sensor.");
        return;
    }

    data.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}