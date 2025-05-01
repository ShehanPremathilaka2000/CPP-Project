#include "../include/ThreadManager.h"
#include <stdexcept>

ThreadManager::ThreadManager(size_t numThreads) 
    : numThreads(numThreads), running(false) {
    if (numThreads <= 0) {
        throw std::invalid_argument("Number of threads must be positive");
    }
}

ThreadManager::~ThreadManager() {
    stop();
}

void ThreadManager::start() {
    if (running) {
        throw std::runtime_error("Thread manager is already running");
    }
    running = true;
    threads.clear();
    
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i]() {
            while (running) {
                // Execute task if available
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(taskMutex);
                    taskCondition.wait(lock, [this]() { 
                        return !running || !taskQueue.empty(); 
                    });
                    
                    if (!running && taskQueue.empty()) {
                        break;
                    }
                    
                    if (!taskQueue.empty()) {
                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }
                }
                
                if (task) {
                    task();
                }
            }
        });
    }
}

void ThreadManager::stop() {
    {
        std::lock_guard<std::mutex> lock(taskMutex);
        running = false;
    }
    taskCondition.notify_all();
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadManager::addTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(taskMutex);
        taskQueue.push(std::move(task));
    }
    taskCondition.notify_one();
}

bool ThreadManager::isRunning() const {
    return running;
}

size_t ThreadManager::getNumThreads() const {
    return numThreads;
}

void ThreadManager::setNumThreads(size_t newNumThreads) {
    if (running) {
        throw std::runtime_error("Cannot change thread count while running");
    }
    numThreads = newNumThreads;
}

size_t ThreadManager::getTaskCount() const {
    std::lock_guard<std::mutex> lock(taskMutex);
    return taskQueue.size();
}

void ThreadManager::waitForCompletion() {
    std::unique_lock<std::mutex> lock(completionMutex);
    while (!taskQueue.empty() || activeThreads > 0) {
        std::this_thread::yield();
    }
}

double ThreadManager::getAverageThreadLoad() const {
    size_t totalLoad = 0;
    for (const auto& load : threadLoads) {
        totalLoad += load;
    }
    return static_cast<double>(totalLoad) / threadLoads.size();
}

size_t ThreadManager::getActiveThreadCount() const {
    return activeThreads;
}

void ThreadManager::processNextTask() {
    std::function<void()> task;
    {
        std::lock_guard<std::mutex> lock(taskMutex);
        if (!taskQueue.empty()) {
            task = std::move(taskQueue.front());
            taskQueue.pop();
        }
    }
    
    if (task) {
        task();
    }
} 