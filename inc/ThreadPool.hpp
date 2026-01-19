#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>

class ThreadPool {
public:
    ThreadPool(size_t numThreads, std::function<void(int)> workerFunc);
    ~ThreadPool();
    void enqueue(int client_fd);
    void shutdown();

private:
    std::vector<std::thread> workers;           // Pool of worker threads
    std::queue<int> tasks;                      // Queue of client_fd to process
    std::mutex queueMutex;                      // Protects the task queue
    std::condition_variable condition;          // Signals workers when tasks arrive
    bool stop;                                  // Flag to stop all workers
    std::function<void(int)> workerFunction;
    void workerThread();
};

#endif // THREADPOOL_HPP