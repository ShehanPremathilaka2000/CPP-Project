#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <queue>

class ThreadManager {
public:
    ThreadManager(size_t numThreads);
    ~ThreadManager();

    // Thread control
    void start();
    void stop();
    void waitForCompletion();  // Bug: Potential deadlock

    // Task management
    void addTask(std::function<void()> task);  // Bug: Not thread-safe
    size_t getTaskCount() const;

    // Thread configuration
    void setNumThreads(size_t numThreads);
    size_t getNumThreads() const;

    // Performance monitoring
    double getAverageThreadLoad() const;  // Bug: Race condition
    size_t getActiveThreadCount() const;

    // Additional methods
    bool isRunning() const;

private:
    // Thread worker
    void workerThread(size_t threadId);  // Bug: Improper synchronization

    // Task queue management
    void processNextTask();  // Bug: Not atomic operation

    // Data members
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    mutable std::mutex taskMutex;
    mutable std::mutex completionMutex;  // Bug: Potential deadlock with queueMutex
    std::condition_variable taskCondition;
    std::atomic<bool> running{false};
    std::atomic<size_t> activeThreads{0};
    size_t numThreads;

    // Performance tracking
    std::vector<std::atomic<size_t>> threadLoads;  // Bug: Not properly synchronized
}; 