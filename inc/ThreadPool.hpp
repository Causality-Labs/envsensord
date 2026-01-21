#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>

template<typename TaskType>

class ThreadPool {
public:
    ThreadPool(size_t numThreads, std::function<void(TaskType)> workerFunc):
    stop(false),
    workerFunction(workerFunc)
    {
        for(size_t i = 0; i < numThreads; i++)
        {
            workers.emplace_back(&ThreadPool::workerThread, this);
        }
    }

    ~ThreadPool()
    {
        shutdown();
    }

    void enqueue(TaskType task_item)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(task_item);
        }
        condition.notify_one();
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();  // Wake up ALL workers
        
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

private:
    std::vector<std::thread> workers;           // Pool of worker threads
    std::queue<TaskType> tasks;                      // Queue of client_fd to process
    std::mutex queueMutex;                      // Protects the task queue
    std::condition_variable condition;          // Signals workers when tasks arrive
    bool stop;                                  // Flag to stop all workers
    std::function<void(TaskType)> workerFunction;

    void workerThread()
    {
        while (true)
        {
            int client_fd;
            {
                // Lock the queue
                std::unique_lock<std::mutex> lock(queueMutex);
                
                // Wait until: tasks available OR stop flag set
                condition.wait(lock, [this]() { 
                    return stop || !tasks.empty(); 
                });
                
                // Exit if stopping and no more tasks
                if (stop && tasks.empty()) {
                    return;
                }
                
                // Get task from queue
                client_fd = tasks.front();
                tasks.pop();
                
            } // Lock automatically released here
            
            workerFunction(client_fd);

        }
    }
};

#endif // THREADPOOL_HPP