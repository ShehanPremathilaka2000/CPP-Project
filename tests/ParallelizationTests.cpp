#include <gtest/gtest.h>
#include "../include/Simulation.h"
#include "../include/Config.h"
#include <chrono>
#include <fstream>
#include <regex>
#include <string>
#include <thread>

// Define a constant thread count for all tests
const size_t FIXED_THREAD_COUNT = 8;

// Test suite for thread-based parallelization verification and benchmarking
class ParallelizationTest : public ::testing::Test {
protected:
    // Create a config with fixed parameters for reproducible benchmarks
    Config createReproducibleConfig(size_t numParticles, size_t numThreads) {
        Config cfg;
        cfg.num_particles = numParticles;
        cfg.field_size = 100.0;
        cfg.initial_threads = numThreads;
        cfg.time_step = 0.01;
        cfg.initial_energy = 100.0;
        cfg.max_energy = 1000.0;
        cfg.particle_radius = 0.5;
        cfg.initial_strength = 50.0;
        cfg.initial_decay_rate = 0.1;
        cfg.field_grid_size = 50;
        cfg.random_seed = 12345; // Fixed seed for reproducibility
        return cfg;
    }

    // Helper function to create particles with deterministic positions and velocities
    void createDeterministicParticles(Simulation& sim, size_t count) {
        for (size_t i = 0; i < count; i++) {
            auto particle = std::make_unique<Particle>(
                (i % 10) - 5.0, // x position
                (i / 10) - 5.0, // y position
                100.0, // energy
                0.5,   // radius
                1000.0 // max energy
            );
            // Set deterministic velocity
            particle->setVelocity(0.1 * (i % 5), -0.1 * (i % 3));
            sim.addParticle(std::move(particle));
        }
    }
};

// Test if thread count is correctly initialized
TEST_F(ParallelizationTest, ThreadCountInitialization) {
    // Create a simulation with a fixed number of threads
    Config config = createReproducibleConfig(0, FIXED_THREAD_COUNT);
    Simulation sim(config);
    
    // Verify simulation is initialized with the correct number of threads
    EXPECT_EQ(sim.getNumThreads(), FIXED_THREAD_COUNT) 
        << "Simulation should be initialized with the specified thread count";
}

// Test if ThreadManager is properly activated
TEST_F(ParallelizationTest, ThreadManagerActivation) {
    // Create a simulation with a fixed number of threads
    Config config = createReproducibleConfig(0, FIXED_THREAD_COUNT);
    Simulation sim(config);
    
    // Verify thread management is working
    bool threadManagerRunning = sim.getThreadManager().isRunning();
    EXPECT_TRUE(threadManagerRunning) 
        << "Thread management should be running after simulation initialization";
}

// Test if particle addition is thread-safe
TEST_F(ParallelizationTest, ThreadSafeParticleAddition) {
    // Create a simulation with multiple threads but no initial particles
    Config config = createReproducibleConfig(0, FIXED_THREAD_COUNT);
    Simulation sim(config);
    
    // Add particles and verify thread safety
    const size_t particleCount = 10000;
    createDeterministicParticles(sim, particleCount);
    EXPECT_EQ(sim.getParticleCount(), particleCount) 
        << "All particles should be added successfully in a thread-safe manner";
}

// Test if the application is properly parallelized by checking if performance improves with more threads
TEST_F(ParallelizationTest, IsParallelized) {
    const int numSteps = 100;
    const size_t particleCount = 5000;
    
    // First run with 1 thread (sequential)
    Config singleThreadConfig = createReproducibleConfig(particleCount, 1);
    Simulation singleThreadSim(singleThreadConfig);
    
    auto startSingle = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numSteps; ++i) {
        singleThreadSim.step();
    }
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto singleTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endSingle - startSingle).count();
    

    const size_t maxThreads = std::thread::hardware_concurrency();
    if (maxThreads <= 1) {
        GTEST_SKIP() << "Skipping parallel test: No parallel execution available";
    }
    
    Config multiThreadConfig = createReproducibleConfig(particleCount, maxThreads);
    Simulation multiThreadSim(multiThreadConfig);
    
    auto startMulti = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numSteps; ++i) {
        multiThreadSim.step();
    }
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto multiTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endMulti - startMulti).count();
    
    std::cout << "Single thread time for " << numSteps << " steps: " << singleTime << "ms" << std::endl;
    std::cout << "Multi thread time (" << maxThreads << " threads) for " 
              << numSteps << " steps: " << multiTime << "ms" << std::endl;
    std::cout << "Speedup: " << (double)singleTime / multiTime << "x" << std::endl;
    
    // Add more stringent performance requirements
    EXPECT_LE(singleTime, 12000) << "Single thread implementation should complete in 12000ms or less";
    
    // Expect reasonable speedup (at least 1.2x with multiple threads)
    // This is a conservative threshold since not all code can be perfectly parallelized
    EXPECT_GT((double)singleTime / multiTime, 2) 
        << "Expected speedup with parallelization. Single thread: " 
        << singleTime << "ms, Multi-thread: " << multiTime << "ms";
}

// Test individual optimized points - Position Updates
TEST_F(ParallelizationTest, ParallelPositionUpdates) {
    const size_t particleCount = 10000;
    
    // Create a simulation with a single thread for comparison
    Config singleConfig = createReproducibleConfig(particleCount, 1);
    Simulation singleSim(singleConfig);
    
    Config multiConfig = createReproducibleConfig(particleCount, FIXED_THREAD_COUNT);
    Simulation multiSim(multiConfig);
    
    // Time single-threaded position updates
    auto startSingle = std::chrono::high_resolution_clock::now();
    singleSim.updatePositions(0.01);
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto singleTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endSingle - startSingle).count();
    
    // Time multi-threaded position updates
    auto startMulti = std::chrono::high_resolution_clock::now();
    multiSim.updatePositions(0.01);
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto multiTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endMulti - startMulti).count();
    
    std::cout << "Position updates single-thread: " << singleTime << "µs" << std::endl;
    std::cout << "Position updates multi-thread: " << multiTime << "µs" << std::endl;
    
    EXPECT_LT(multiTime, singleTime) << "Multi-threaded position updates should be faster than single-threaded";
}

// Test individual optimized points - Force Application
TEST_F(ParallelizationTest, ParallelForceApplication) {
    const size_t particleCount = 10000;
    
    // Create simulations for single and multi-threaded tests
    Config singleConfig = createReproducibleConfig(particleCount, 1);
    Simulation singleSim(singleConfig);

    Config multiConfig = createReproducibleConfig(particleCount, FIXED_THREAD_COUNT);
    Simulation multiSim(multiConfig);
    
    // Time single-threaded force application
    auto startSingle = std::chrono::high_resolution_clock::now();
    singleSim.applyForces(0.01);
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto singleTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endSingle - startSingle).count();
    
    // Time multi-threaded force application
    auto startMulti = std::chrono::high_resolution_clock::now();
    multiSim.applyForces(0.01);
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto multiTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endMulti - startMulti).count();
    
    std::cout << "Force application single-thread: " << singleTime << "µs" << std::endl;
    std::cout << "Force application multi-thread: " << multiTime << "µs" << std::endl;
    
    EXPECT_LT(multiTime, singleTime) << "Multi-threaded force application should be faster than single-threaded";

}

// Test individual optimized points - Energy Calculation
TEST_F(ParallelizationTest, ParallelEnergyCalculation) {
    const size_t particleCount = 10000;
    
    Config config = createReproducibleConfig(particleCount, FIXED_THREAD_COUNT);
    Simulation sim(config);
    
    // Create single-threaded simulation for comparisonmaxThreads
    Config singleConfig = createReproducibleConfig(particleCount, 1);
    Simulation singleSim(singleConfig);
    
    auto startSingle = std::chrono::high_resolution_clock::now();
    double energySingle = singleSim.getTotalEnergy();
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto singleTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endSingle - startSingle).count();
    
    // Now use multi-threaded simulation
    auto startMulti = std::chrono::high_resolution_clock::now();
    double energyMulti = sim.getTotalEnergy();
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto multiTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endMulti - startMulti).count();
    
    std::cout << "Energy calculation single-thread: " << singleTime << "µs" << std::endl;
    std::cout << "Energy calculation multi-thread: " << multiTime << "µs" << std::endl;
    
    // Verify results are the same (within floating-point precision)
    EXPECT_NEAR(energySingle, energyMulti, 1e-10) << "Energy calculations should yield identical results";
    
    EXPECT_LT(multiTime, singleTime) << "Multi-threaded energy calculation should be faster";
}

// Test that collision handling is correctly parallelized
TEST_F(ParallelizationTest, ParallelCollisionHandling) {
    // Set up a controlled scenario with many particles placed to cause collisions
    const size_t particleCount = 1000;
    
    Config singleConfig = createReproducibleConfig(0, 1); // Start with 0 particles
    Simulation singleSim(singleConfig);

    Config multiConfig = createReproducibleConfig(0, FIXED_THREAD_COUNT); // Start with 0 particles
    Simulation multiSim(multiConfig);
    
    // Create particles in a grid pattern with overlapping radiuses to guarantee collisions
    createDeterministicParticles(singleSim, particleCount);
    createDeterministicParticles(multiSim, particleCount);
    
    // Time single-threaded collision handling
    auto startSingle = std::chrono::high_resolution_clock::now();
    singleSim.handleCollisions();
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto singleTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endSingle - startSingle).count();
    
    // Time multi-threaded collision handling
    auto startMulti = std::chrono::high_resolution_clock::now();
    multiSim.handleCollisions();
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto multiTime = std::chrono::duration_cast<std::chrono::microseconds>(
        endMulti - startMulti).count();
    
    std::cout << "Collision handling single-thread: " << singleTime << "µs" << std::endl;
    std::cout << "Collision handling multi-thread: " << multiTime << "µs" << std::endl;
    
    EXPECT_GT(singleTime, multiTime) << "Multi-threaded time should be less than Single-threaded time";
}
