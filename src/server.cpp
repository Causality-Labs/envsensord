#include <iostream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <thread>
#include "my_socket_lib.hpp"
#include "logger.hpp"
#include "bme280.hpp"
#include "SSNP.hpp"
#include "ThreadPool.hpp"

StdLogger stdlogger("HW_Server");
BME280::BME280 bme280;
Server server;


int getSensorData(ssnp::SsnpRequestType& request, BME280::BME280& sensor, BME280::SensorData& data);

void handleClient(int client_fd);

int main (void)
{
    int ret{};

    ret = bme280.init();
    if (ret != 0) {
        stdlogger.error("Was not able to initialize bme280 sensor.");
        return -1;
    }

    stdlogger.info("Initialized bme280 sensor.");

    ret = server.connect_to_port(3500, 25);
    if (ret < 0) {
        stdlogger.error("Failed to start server");
        return 1;
    }

    stdlogger.info("Server listening on port 3500...");

    ThreadPool thread_pool(4, handleClient);

    while (1)
    {
        int client_fd = server.wait_for_connection();
        if (client_fd <= 0) {
            stdlogger.error("Failed to connect to a client");
            continue;
        }

        stdlogger.info("Client connected");
        // Put client_fd in ThreadPool task queue.

        #if 0
        std::string client_request{};
        ret = server.receive_request(client_fd, client_request);

        if (ret < 0) {
            stdlogger.error("Failed to recieve data from client.");
            close(client_fd);
            continue;
        }

        stdlogger.info("Client Request: " + client_request);

        // Parse data recieved
        ret = Parser.parseRequest(client_request);
        if (ret < 0 || Parser.req_type.invalid_req == true) {
            stdlogger.error("Invalid Client Request");
            close(client_fd);
            continue;
        }

        BME280::SensorData sensor_data;
        ret = getSensorData(Parser.req_type, bme280, sensor_data);
        if (ret != 0) {
            stdlogger.error("Failed to read sensor data");
            close(client_fd);
            continue;
        }

        std::string client_response{};
        Parser.buildResponse(sensor_data, client_response);

        ret = server.send_data(client_fd, client_response);
        if (ret == -1) {
            stdlogger.error("Failed to send data to server");
            close(client_fd);
            continue;
        }

        stdlogger.info("Response sent to client");
        close(client_fd);
        #endif
    }

    return 0;
}

int getSensorData(ssnp::SsnpRequestType& request, BME280::BME280& sensor, BME280::SensorData& data)
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

void handleClient(int client_fd)
{

    ssnp::Ssnp Parser;
    int ret;

    while (true)
    {
        // Recieve task data (client_fd,)
        std::string client_request{};
        ret = server.receive_request(client_fd, client_request);

        if (ret < 0) {
            stdlogger.error("Failed to recieve data from client.");
            close(client_fd);
            continue;
        }

        stdlogger.info("Client Request: " + client_request);

        // Parse data recieved
        ret = Parser.parseRequest(client_request);
        if (ret < 0 || Parser.req_type.invalid_req == true) {
            stdlogger.error("Invalid Client Request");
            close(client_fd);
            continue;
        }

        BME280::SensorData sensor_data;
        ret = getSensorData(Parser.req_type, bme280, sensor_data);
        if (ret != 0) {
            stdlogger.error("Failed to read sensor data");
            close(client_fd);
            continue;
        }

        std::string client_response{};
        Parser.buildResponse(sensor_data, client_response);

        ret = server.send_data(client_fd, client_response);
        if (ret == -1) {
            stdlogger.error("Failed to send data to server");
            close(client_fd);
            continue;
        }

        stdlogger.info("Response sent to client");
        close(client_fd);
    }
}