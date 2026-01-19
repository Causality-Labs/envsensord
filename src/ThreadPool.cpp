#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t numThreads, std::function<void(int)> workerFunc) :
stop(false),
workerFunction(workerFunc)
{
    for(size_t i = 0; i < numThreads; i++)
    {
        workers.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool() {}

void ThreadPool::workerThread()
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