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
                  std::function<void(SensorType&, DataType&)> readFunc,
                  int updateIntervalMs = 1000)
    : sensor(sensor),
      running(false),
      updateInterval(updateIntervalMs),
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

    void getData(DataType& data)
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        data = cachedData;
    }

private:
    SensorType& sensor;
    DataType cachedData;  
    std::mutex dataMutex;
    std::thread updateThread;
    std::atomic<bool> running;
    int updateInterval;
    std::function<void(SensorType&, DataType&)> readFunction;
    
    void updateLoop()
    {
        while (running) {
            DataType freshData;
            
            // Call user-provided read function
            readFunction(sensor, freshData);
            
            // Update cache under lock
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                cachedData = freshData;
            }
            
            // Sleep for update interval
            std::this_thread::sleep_for(std::chrono::milliseconds(updateInterval));
        }
    }
};

#endif // SENSOR_MANAGER_HPP