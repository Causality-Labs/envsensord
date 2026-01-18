#include <iostream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include "my_socket_lib.hpp"
#include "logger.hpp"
#include "bme280.hpp"
#include "SSNP.hpp"

int getSensorData(ssnp::SsnpRequestType& request, bme280::BME280& sensor, bme280::SensorData& data);


int main (void)
{

    StdLogger logger("HW_Server");
    bme280::BME280 bme280;
    Server server;
    ssnp::Ssnp Parser;
    int client_fd{};
    int ret{};

    ret = bme280.init();
    if (ret != 0) {
        logger.error("Was not able to initialize bme280 sensor.");
        return -1;
    }

    logger.info("Initialized bme280 sensor.");

    ret = server.connect_to_port(3500, 25);
    if (ret < 0) {
        logger.error("Failed to start server");
        return 1;
    }

    logger.info("Server listening on port 3500...");


    while (1)
    {
        client_fd = server.wait_for_connection();
        if (client_fd <= 0) {
            logger.error("Failed to connect to a client");
            continue;
        }

        logger.info("Client connected");

        std::string client_request{};
        ret = server.receive_request(client_fd, client_request);

        if (ret < 0) {
            logger.error("Failed to recieve data from client.");
            close(client_fd);
            continue;
        }

        logger.info("Client Request: " + client_request);

        // Parse data recieved
        ret = Parser.parseRequest(client_request);
        if (ret < 0 || Parser.req_type.invalid_req == true) {
            logger.error("Invalid Client Request");
            close(client_fd);
            continue;
        }

        bme280::SensorData sensor_data;
        ret = getSensorData(Parser.req_type, bme280, sensor_data);
        if (ret != 0) {
            logger.error("Failed to read sensor data");
            close(client_fd);
            continue;
        }

        std::string client_response{};
        Parser.buildResponse(sensor_data, client_response);

        ret = server.send_data(client_fd, client_response);
        if (ret == -1) {
            logger.error("Failed to send data to server");
            close(client_fd);
            continue;
        }

        logger.info("Response sent to client");
        close(client_fd);
    }

    return 0;
}

int getSensorData(ssnp::SsnpRequestType& request, bme280::BME280& sensor, bme280::SensorData& data)
{
    int ret{};

    if (request.temp_req) {
        ret = sensor.readTemperature(data.temperature);
        if (ret != 0) {
            return -1;
        }
    }

    if (request.hum_req) {
        ret = sensor.readHumidity(data.humidity);
        if (ret != 0) {
            return -1;
        }
    }

    if (request.press_req) {
        ret = sensor.readPressure(data.pressure);
        if (ret != 0) {
            return -1;
        }
    }

    auto now = std::chrono::system_clock::now();
    data.timestamp = std::chrono::system_clock::to_time_t(now);

    return 0;
}