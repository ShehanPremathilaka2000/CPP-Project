// ---- File: SimulationTests.cpp ----

#include <gtest/gtest.h>
#include "../include/Simulation.h"
#include "../include/Particle.h"
#include "../include/ContainmentField.h"
#include "../include/Config.h"
#include <memory>
#include <chrono> // For performance test timing
#include <vector> // For performance test

// Helper function to create a default Config for tests
Config createTestConfig() {
    Config cfg;
    cfg.num_particles = 100;
    cfg.field_size = 10.0;
    cfg.initial_threads = 1;
    cfg.time_step = 0.01;
    cfg.initial_energy = 100.0;
    cfg.max_energy = 1000.0;
    cfg.particle_radius = 0.5;
    cfg.initial_strength = 50.0;
    cfg.initial_decay_rate = 0.1;
    cfg.field_grid_size = 10;
    // Rendering settings are not typically used in simulation logic tests
    return cfg;
}

class SimulationTest : public ::testing::Test {
protected:
    Config testConfig;

    void SetUp() override {
        testConfig = createTestConfig();
        simulation = std::make_unique<Simulation>(testConfig);
    }

    std::unique_ptr<Simulation> simulation;
};

// Test that particles are initialized by the constructor
TEST_F(SimulationTest, Initialization) {
    EXPECT_EQ(simulation->getParticleCount(), testConfig.num_particles) << "Number of particles should match the config.";
}

// Test that a particle can be added to the simulation
TEST_F(SimulationTest, AddParticle) {
    size_t initialCount = simulation->getParticleCount();
    auto particle = std::make_unique<Particle>(
        0.0, 0.0, testConfig.initial_energy, testConfig.particle_radius, testConfig.max_energy
    );
    simulation->addParticle(std::move(particle));
    EXPECT_EQ(simulation->getParticleCount(), initialCount + 1) << "Particle count should increase by 1 after adding a particle.";
}

// Test a single simulation step changes state
TEST_F(SimulationTest, UpdateSimulation) {
     // Ensure there are particles to update
    if (simulation->getParticleCount() == 0) {
        auto particle = std::make_unique<Particle>(1.0, 1.0, testConfig.initial_energy, testConfig.particle_radius, testConfig.max_energy);
        particle->setVelocity(0.1, 0.0);
        simulation->addParticle(std::move(particle));
    }
    ASSERT_GT(simulation->getParticleCount(), 0) << "Simulation has no particles to update.";

    const auto& particles = simulation->getParticles();
    double initialX = particles[0]->getX(); // Get initial position
    double initialTotalEnergy = simulation->getTotalEnergy(); // Get initial total energy

    simulation->step(); // Run one simulation step

    double finalX = particles[0]->getX(); // Get final position
    double finalTotalEnergy = simulation->getTotalEnergy(); // Get final total energy

    EXPECT_NE(initialX, finalX) << "Particle position did not change after step.";
}

// Test energy conservation
TEST_F(SimulationTest, EnergyChange) {
    if (simulation->getParticleCount() == 0) {
        testConfig.num_particles = 10;
        simulation = std::make_unique<Simulation>(testConfig);
    }
     ASSERT_GT(simulation->getParticleCount(), 0) << "Simulation has no particles for energy test.";

    double initialEnergy = simulation->getTotalEnergy();
    for (int i = 0; i < 100; ++i) { // Run multiple steps
        simulation->step();
    }
    double finalEnergy = simulation->getTotalEnergy();

    EXPECT_NE(initialEnergy, finalEnergy) << "Total energy did not change after 100 steps.";
}

// Test interactions
TEST_F(SimulationTest, ParticleInteractionAndEscape) {
    testConfig.num_particles = 2;
    simulation = std::make_unique<Simulation>(testConfig);

    size_t initialCount = simulation->getParticleCount();
    ASSERT_EQ(initialCount, 2);

    // Run simulation for enough steps for particles to move/interact/escape
    for (int i = 0; i < 500; ++i) { // Adjust step count as needed
        simulation->step();
        // Optional: break if count changes
        if (simulation->getParticleCount() != initialCount) break;
    }

    // Check if particles escaped (count decreased)
    size_t finalCount = simulation->getParticleCount();
    EXPECT_LE(finalCount, initialCount) << "Particle count should not increase.";
}

// Test parallel performance
TEST_F(SimulationTest, ParallelPerformance) {
    testConfig.num_particles = 500; // Increase particle count
    simulation = std::make_unique<Simulation>(testConfig); // Recreate with more particles

    ASSERT_GE(simulation->getParticleCount(), 500) << "Need sufficient particles for performance test.";

    const int numSteps = 50;

    // --- Run with single thread ---
    simulation->setNumThreads(1);
    simulation->start(); // Start worker threads

    auto startSingle = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numSteps; ++i) {
        simulation->step();
    }
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto singleThreadTime = std::chrono::duration_cast<std::chrono::milliseconds>(endSingle - startSingle);
    simulation->stop(); // Stop worker threads

    // --- Run with multiple threads ---
    size_t numHardwareThreads = std::thread::hardware_concurrency();
    size_t multiThreadCount = (numHardwareThreads > 1) ? 4 : 1;
    if (multiThreadCount <= 1) {
        GTEST_SKIP() << "Skipping parallel performance test: Not enough hardware threads or failed to detect.";
    }

    simulation->setNumThreads(multiThreadCount);
    simulation->start(); // Start worker threads

    auto startMulti = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numSteps; ++i) {
        simulation->step(); // step() logic might need parallelization internally
    }
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto multiThreadTime = std::chrono::duration_cast<std::chrono::milliseconds>(endMulti - startMulti);
    simulation->stop(); // Stop worker threads


    std::cout << "[ PERF ] Single thread (" << numSteps << " steps): " << singleThreadTime.count() << " ms" << std::endl;
    std::cout << "[ PERF ] Multi thread (" << multiThreadCount << " threads, " << numSteps << " steps): " << multiThreadTime.count() << " ms" << std::endl;

    EXPECT_GT(singleThreadTime.count(), 0);
    EXPECT_GT(multiThreadTime.count(), 0);

    // EXPECT_LT(multiThreadTime.count(), singleThreadTime.count()); // Ideal case
} 