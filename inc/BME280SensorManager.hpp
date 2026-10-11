#ifndef BME280SENSORMANAGER_HPP
#define BME280SENSORMANAGER_HPP

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <thread>

#include "BME280.hpp"

class BME280SensorManager {
public:
    static constexpr std::chrono::milliseconds DEFAULT_UPDATE_INTERVAL{100};

    explicit BME280SensorManager(BME280& sensor,
                                 std::chrono::milliseconds updateInterval = DEFAULT_UPDATE_INTERVAL);

    BME280SensorManager(const BME280SensorManager&) = delete;
    BME280SensorManager& operator=(const BME280SensorManager&) = delete;

    int init();
    int read(BME280Data& out);

private:
    struct SensorDataCache
    {
        BME280Data data{};
        std::chrono::steady_clock::time_point readAt{};
        bool validSample = false;
    };

    void acquisitionWorker(std::stop_token stopToken);

    BME280& m_sensor;
    std::chrono::milliseconds m_updateInterval;
    bool m_initialized = false;
    std::condition_variable_any m_cv;

    std::mutex m_cacheMutex;
    SensorDataCache m_cache;

    std::jthread m_thread;
};

#endif // BME280SENSORMANAGER_HPP
