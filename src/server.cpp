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
#include "SensorManager.hpp"


int getSensorData(ssnp::SsnpRequestType& request, BME280::BME280& sensor, BME280::SensorData& data);
void handleClientCallback(int client_fd);
void readBME280DataCallback(BME280::BME280& sensor, BME280::SensorData& data);

StdLogger stdlogger("HW_Server");
BME280::BME280 bme280;
Server server;
SensorManager<BME280::BME280, BME280::SensorData> sensorMgr(bme280,
                                                            readBME280DataCallback,
                                                            5000);

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

    ThreadPool<int> thread_pool(4, handleClientCallback);
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

void handleClientCallback(int client_fd)
{

    ssnp::Ssnp Parser;
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
    ret = Parser.parseRequest(client_request);
    if (ret < 0 || Parser.req_type.invalid_req == true) {
        stdlogger.error("Invalid Client Request");
        close(client_fd);
        return;
    }

    BME280::SensorData sensor_data;
    sensorMgr.getData(sensor_data); 

    std::string client_response{};
    Parser.buildResponse(sensor_data, client_response);

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
    sensor.readTemperature(data.temperature);
    sensor.readPressure(data.pressure);
    sensor.readHumidity(data.humidity);
    data.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    return;
}