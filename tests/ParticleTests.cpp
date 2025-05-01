#include <gtest/gtest.h>
#include "../include/Particle.h"
#include <thread>
#include <memory>
#include <vector>

class ParticleTest : public ::testing::Test {
protected:
    void SetUp() override {
        particle = std::make_unique<Particle>(0.0, 0.0, 100.0, 1.0, 1000.0);
    }

    std::unique_ptr<Particle> particle;
};

// Test position and velocity
TEST_F(ParticleTest, PositionAndVelocity) {
    particle->setPosition(1.0, 2.0);
    particle->setVelocity(4.0, 5.0);
    
    EXPECT_DOUBLE_EQ(particle->getX(), 1.0);
    EXPECT_DOUBLE_EQ(particle->getY(), 2.0);
    
    EXPECT_DOUBLE_EQ(particle->getVX(), 4.0);
    EXPECT_DOUBLE_EQ(particle->getVY(), 5.0);
}

// Test energy management
TEST_F(ParticleTest, EnergyManagement) {
    const double initialEnergy = particle->getEnergy();
    
    // Add energy
    particle->addEnergy(50.0);
    EXPECT_DOUBLE_EQ(particle->getEnergy(), initialEnergy + 50.0);
    
    // Set energy
    particle->setEnergy(200.0);
    EXPECT_DOUBLE_EQ(particle->getEnergy(), 200.0);

    // Test energy bounds (upper)
    particle->setEnergy(1500.0);
    EXPECT_LE(particle->getEnergy(), particle->getMaxEnergy()); // Check it's capped
    EXPECT_DOUBLE_EQ(particle->getEnergy(), 1000.0); // Check it's exactly max

    // Test energy bounds (lower)
    particle->setEnergy(-50.0);
    EXPECT_GE(particle->getEnergy(), 0.0); // Check it's non-negative
    EXPECT_DOUBLE_EQ(particle->getEnergy(), 0.0); // Check it's exactly zero

    // Test addEnergy bounds
    particle->setEnergy(980.0);
    particle->addEnergy(50.0);
    EXPECT_LE(particle->getEnergy(), particle->getMaxEnergy());
    EXPECT_DOUBLE_EQ(particle->getEnergy(), 1000.0);

    particle->setEnergy(20.0);
    particle->addEnergy(-50.0);
    EXPECT_GE(particle->getEnergy(), 0.0);
    EXPECT_DOUBLE_EQ(particle->getEnergy(), 0.0);
}

// Test collision
TEST_F(ParticleTest, Collision) {
    auto p1 = std::make_unique<Particle>(0.0, 0.0, 100.0, 1.0, 1000.0);
    auto p2 = std::make_unique<Particle>(2.0, 0.0, 100.0, 1.0, 1000.0);

    // Initial velocities: p1 moves right (+x), p2 moves left (-x)
    p1->setVelocity(5.0, 0.0); // p1 velocity (5, 0)
    p2->setVelocity(-3.0, 1.0); // p2 velocity (-3, 1)

    // Store initial states
    const double p1_initial_vx = p1->getVX();
    const double p1_initial_vy = p1->getVY();
    const double p1_initial_energy = p1->getEnergy();

    const double p2_initial_vx = p2->getVX();
    const double p2_initial_vy = p2->getVY();
    const double p2_initial_energy = p2->getEnergy();

    // Perform collision
    p1->collide(*p2);

    // Check if velocities swapped correctly
    EXPECT_DOUBLE_EQ(p1->getVX(), p2_initial_vx); // p1's final vx should be p2's initial vx
    EXPECT_DOUBLE_EQ(p1->getVY(), p2_initial_vy); // p1's final vy should be p2's initial vy

    EXPECT_DOUBLE_EQ(p2->getVX(), p1_initial_vx); // p2's final vx should be p1's initial vx
    EXPECT_DOUBLE_EQ(p2->getVY(), p1_initial_vy); // p2's final vy should be p1's initial vy

    // Check if internal energy is conserved for *each* particle
    EXPECT_DOUBLE_EQ(p1->getEnergy(), p1_initial_energy);
    EXPECT_DOUBLE_EQ(p2->getEnergy(), p2_initial_energy);
}

// Test collision detection
TEST_F(ParticleTest, CollisionDetection) {
    auto p1 = std::make_unique<Particle>(0.0, 0.0, 100.0, 1.0, 1000.0);
    auto p2 = std::make_unique<Particle>(2.0, 0.0, 100.0, 1.0, 1000.0);
    
    EXPECT_TRUE(p1->isColliding(*p2));
    
    p2->setPosition(2.1, 0.0);
    EXPECT_FALSE(p1->isColliding(*p2));
    
    p2->setPosition(3.0, 3.0);
    EXPECT_FALSE(p1->isColliding(*p2));
}

// Test thread safety
TEST_F(ParticleTest, ThreadSafety) {
    const int numThreads = 4;
    const int numIterations = 1000;
    std::vector<std::thread> threads;

    // Initial energy is 100. Max is 1000. Radius is 1.0.
    double initialEnergy = particle->getEnergy();

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < numIterations; ++j) {
                particle->addEnergy(1.0); // Adds energy
                particle->setPosition(static_cast<double>(j % 10), static_cast<double>(j % 10)); // Sets position
                particle->setVelocity(static_cast<double>(i), static_cast<double>(j)); // Sets velocity
                // Read operations
                particle->getX();
                particle->getY();
                particle->getVX();
                particle->getVY();
                particle->getEnergy();
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Energy should be within bounds
    EXPECT_LE(particle->getEnergy(), particle->getMaxEnergy());
    EXPECT_DOUBLE_EQ(particle->getEnergy(), particle->getMaxEnergy()); // Should be exactly max energy
    
    // Position should be valid
    EXPECT_GE(particle->getX(), 0.0);
    EXPECT_LE(particle->getX(), 9.0); // Max value set inside loop
    EXPECT_GE(particle->getY(), 0.0);
    EXPECT_LE(particle->getY(), 9.0); // Max value set inside loop
}

// Test memory management
TEST_F(ParticleTest, MemoryManagement) {
    const int numParticles = 1000;
    std::vector<std::unique_ptr<Particle>> particles;
    
    for (int i = 0; i < numParticles; ++i) {
        particles.push_back(std::make_unique<Particle>(0.0, 0.0, 100.0, 1.0, 1000.0));
    }
    
    // All particles should be properly initialized
    for (const auto& p : particles) {
        EXPECT_NE(p, nullptr);
        // Check initial energy (should be clamped between 0 and max)
        EXPECT_GE(p->getEnergy(), 0.0);
        EXPECT_LE(p->getEnergy(), p->getMaxEnergy());
    }
} 