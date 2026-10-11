#include "BME280SensorManager.hpp"

BME280SensorManager::BME280SensorManager(BME280& sensor, std::chrono::milliseconds updateInterval):
    m_sensor(sensor),
    m_updateInterval(updateInterval)
{

}

int BME280SensorManager::init()
{
    if (m_initialized)
    {
        return -1;
    }

    if (m_updateInterval < MIN_UPDATE_INTERVAL || m_updateInterval > MAX_UPDATE_INTERVAL)
    {
        return -1;
    }

    if (m_sensor.init() != 0)
    {
        return -1;
    }

    BME280Data probe{};
    if (m_sensor.read(probe) != 0)
    {
        return -1;
    }

    // Fill the cache now so read() has data before the worker's first update
    {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_cache.data = probe;
        m_cache.readAt = std::chrono::steady_clock::now();
        m_cache.validSample = true;
    }

    m_thread = std::jthread([this](std::stop_token stopToken) { acquisitionWorker(stopToken); });
    m_initialized = true;
    return 0;
}

int BME280SensorManager::read(BME280Data& out)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);

    if (!m_cache.validSample)
    {
        return -1;
    }

    out = m_cache.data;
    return 0;
}

void BME280SensorManager::acquisitionWorker(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
    {
        // Interruptible sleep: wakes early on stop request
        {
            std::unique_lock<std::mutex> lock(m_cacheMutex);
            m_cv.wait_for(lock, stopToken, m_updateInterval, [] { return false; });
        }

        if (stopToken.stop_requested())
        {
            break;
        }

        BME280Data data{};
        if (m_sensor.read(data) == 0)
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_cache.data = data;
            m_cache.readAt = std::chrono::steady_clock::now();
            m_cache.validSample = true;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_cache.validSample = false;
        }
    }
}
