#ifndef SENSOR_MANAGER_HPP
#define SENSOR_MANAGER_HPP

#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

template<typename SensorType, typename DataType>
class SensorManager {
public:
    SensorManager(SensorType& sensor, 
                  std::function<void(SensorType&, DataType&)> readFunc)
    : sensor(sensor),
      running(false),
      updateInterval(1000),
      readFunction(readFunc)
    {}

    ~SensorManager()
    {
        stop();
    }

    void start()
    {
        running = true;
        updateThread = std::thread(&SensorManager::updateLoop, this);
    }

    void stop()
    {
        running = false;
        if (updateThread.joinable()) {
            updateThread.join();
        }
    }

    bool setInterval(int interval)
    {
        if (running == true)
            return false;

        if (interval <= 0)
            return false;

        updateInterval = interval;

        return true;
    }

    void getData(DataType& data)
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        data = cachedData;
    }

private:
    SensorType& sensor;
    DataType cachedData;
    DataType freshData;
    std::mutex dataMutex;
    std::thread updateThread;
    std::atomic<bool> running;
    int updateInterval;
    std::function<void(SensorType&, DataType&)> readFunction;

    void updateLoop()
    {
        while (running)
        {
            readFunction(sensor, freshData);
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                cachedData = freshData;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(updateInterval));
        }
    }
};

#endif // SENSOR_MANAGER_HPP