#pragma once

#include "Particle.h"
#include "ContainmentField.h"
#include "ThreadManager.h"
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct Config;

class Simulation {
public:
    Simulation(const Config& config);
    ~Simulation();

    // Initialization
    void initializeParticles(const Config& config);
    void setContainmentField(std::unique_ptr<ContainmentField> field);

    // Simulation control
    void start();
    void stop();
    void step();  // Bug: Not thread-safe

    // Particle management
    void addParticle(std::unique_ptr<Particle> particle);
    void removeEscapedParticles();  // Bug: Memory leak
    size_t getParticleCount() const;
    const std::vector<std::unique_ptr<Particle>>& getParticles() const;  // Added getter method

    // Energy management
    double getTotalEnergy() const;  // Bug: Race condition

    // Thread management
    void setNumThreads(size_t numThreads);
    size_t getNumThreads() const;
    const ThreadManager& getThreadManager() const { return *threadManager; }

    void updatePositions(double timeStep); 
    void applyForces(double timeStep);
    void handleCollisions();

private:
    // Core simulation methods
      // Bug: Deadlock potential
      // Bug: Incorrect force calculation

    // Thread worker
    void workerThread(size_t threadId);  // Bug: Improper thread synchronization

    // Data members
    std::vector<std::unique_ptr<Particle>> particles;
    std::unique_ptr<ContainmentField> containmentField;
    std::unique_ptr<ThreadManager> threadManager;
    double fieldSize;  // Added fieldSize member
    const double timeStep;  // Added timeStep member

    // Threading
    std::vector<std::thread> workerThreads;
    std::mutex simulationMutex;
    std::mutex particleMutex;  // Bug: Potential deadlock with simulationMutex
    std::condition_variable cv;
    std::atomic<bool> running{false};
    size_t numThreads;
}; 