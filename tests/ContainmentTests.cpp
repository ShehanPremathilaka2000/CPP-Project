#include <gtest/gtest.h>
#include "../include/ContainmentField.h"
#include "../include/Particle.h"
#include "../include/Config.h"
#include <memory>

class ContainmentFieldTest : public ::testing::Test {
protected:
    Config cfg;

    void SetUp() override {
        // Set up a default config for tests
        cfg.field_size = 10.0;          // Field size for tests
        cfg.initial_strength = 50.0;    // Example strength
        cfg.initial_decay_rate = 0.1;   // Example decay rate
        cfg.field_grid_size = 10;       // Smaller grid for tests maybe?
        cfg.particle_radius = 0.5;      // Example radius
        cfg.max_energy = 1000.0;        // Example max energy

        // Create field and particle using the config
        field = std::make_unique<ContainmentField>(cfg);
        particle = std::make_unique<Particle>(0.0, 0.0, 100.0, cfg.particle_radius, cfg.max_energy);
    }

    std::unique_ptr<ContainmentField> field;
    std::unique_ptr<Particle> particle;
    const double TOLERANCE = 1e-9;
};

// Test field size
TEST_F(ContainmentFieldTest, FieldSize) {
    EXPECT_DOUBLE_EQ(field->getSize(), cfg.field_size);
}

// Test particle containment
TEST_F(ContainmentFieldTest, ParticleContainment) {
    double sizeHalf = cfg.field_size / 2.0;

    // Test inside field
    particle->setPosition(sizeHalf * 0.5, sizeHalf * 0.5);
    EXPECT_TRUE(field->isParticleContained(*particle));
    
    // Test outside field
    particle->setPosition(sizeHalf * 1.1, sizeHalf * 1.1);
    EXPECT_FALSE(field->isParticleContained(*particle));
    
    // Test boundary
    particle->setPosition(sizeHalf, sizeHalf);
    EXPECT_FALSE(field->isParticleContained(*particle));
    
    // Test just inside field
    particle->setPosition(sizeHalf * 0.99, sizeHalf * 0.99);
    EXPECT_TRUE(field->isParticleContained(*particle));
    
     // Test negative coordinates inside
    particle->setPosition(-sizeHalf * 0.5, -sizeHalf * 0.5);
    EXPECT_TRUE(field->isParticleContained(*particle));
    
    // Test negative coordinates outside
    particle->setPosition(-sizeHalf * 1.1, -sizeHalf * 1.1);
    EXPECT_FALSE(field->isParticleContained(*particle));
}

// Test containment force
TEST_F(ContainmentFieldTest, ContainmentForce) {
    const double fieldStrength = cfg.initial_strength; // 50.0
    const double halfSize = cfg.field_size / 2.0;      // 5.0

    // --- Test cases strictly INSIDE the field ---

    // 1. Particle exactly at the center (0, 0)
    // minDistToEdge = min(5-0, 5-0) = 5. Force = 50 * (1 - 5/5) = 0
    particle->setPosition(0.0, 0.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force at center should be zero.";

    // 2. Particle inside, near center (e.g., 1, 2)
    // minDistToEdge = min(5-1, 5-2) = min(4, 3) = 3. Force = 50 * (1 - 3/5) = 50 * 0.4 = 20.0
    particle->setPosition(1.0, 2.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 20.0, TOLERANCE) << "Force calculation failed for (1, 2).";

    // 3. Particle inside, closer to one edge (e.g., 4.0, 1.0) -> closer to x=5 edge
    // minDistToEdge = min(5-4, 5-1) = min(1, 4) = 1. Force = 50 * (1 - 1/5) = 50 * 0.8 = 40.0
    particle->setPosition(4.0, 1.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 40.0, TOLERANCE) << "Force calculation failed for (4, 1).";

    // 4. Particle inside, closer to a different edge (e.g., -1.0, -4.5) -> closer to y=-5 edge
    // minDistToEdge = min(5-|-1|, 5-|-4.5|) = min(4, 0.5) = 0.5. Force = 50 * (1 - 0.5/5) = 50 * (1 - 0.1) = 50 * 0.9 = 45.0
    particle->setPosition(-1.0, -4.5);
    EXPECT_NEAR(field->getContainmentForce(*particle), 45.0, TOLERANCE) << "Force calculation failed for (-1, -4.5).";

    // 5. Particle inside, near a corner (e.g., 4.0, 4.0)
    // minDistToEdge = min(5-4, 5-4) = min(1, 1) = 1. Force = 50 * (1 - 1/5) = 50 * 0.8 = 40.0
    particle->setPosition(4.0, 4.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 40.0, TOLERANCE) << "Force calculation failed for (4, 4).";

    // 6. Particle very close to an edge (e.g., 4.99, 0.0)
    // minDistToEdge = min(5-4.99, 5-0) = min(0.01, 5) = 0.01. Force = 50 * (1 - 0.01/5) = 50 * (1 - 0.002) = 50 * 0.998 = 49.9
    particle->setPosition(halfSize - 0.01, 0.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), fieldStrength * (1.0 - 0.01 / halfSize), TOLERANCE) << "Force calculation failed near edge.";


    // --- Test cases ON or OUTSIDE the boundary ---

    // 7. Particle exactly on an edge (X-axis)
    particle->setPosition(halfSize, 0.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force on X edge should be zero.";

    // 8. Particle exactly on an edge (Y-axis, negative)
    particle->setPosition(2.0, -halfSize);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force on negative Y edge should be zero.";

    // 9. Particle exactly on a corner
    particle->setPosition(-halfSize, halfSize);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force on corner should be zero.";

    // 10. Particle outside the field (X-axis)
    particle->setPosition(halfSize + 1.0, 0.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force outside (X) should be zero.";

    // 11. Particle outside the field (Y-axis)
    particle->setPosition(0.0, -halfSize - 1.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force outside (Y) should be zero.";

    // 12. Particle outside the field (diagonally)
    particle->setPosition(halfSize + 1.0, halfSize + 1.0);
    EXPECT_NEAR(field->getContainmentForce(*particle), 0.0, TOLERANCE) << "Force outside (diagonal) should be zero.";
}

// Test field strength getter/setter
TEST_F(ContainmentFieldTest, FieldStrength) {
    const double initialStrength = field->getFieldStrength(); // Reads initial strength
    EXPECT_DOUBLE_EQ(initialStrength, cfg.initial_strength);
    
    field->setFieldStrength(200.0);
    EXPECT_DOUBLE_EQ(field->getFieldStrength(), 200.0);
    
    // Force should scale with field strength
    particle->setPosition(0.0, 0.0);
    const double force1 = field->getContainmentForce(*particle);
    EXPECT_DOUBLE_EQ(force1, 200.0); // Force at center = new strength
    
    field->setFieldStrength(400.0);
    const double force2 = field->getContainmentForce(*particle);
    EXPECT_DOUBLE_EQ(force2, 400.0); // Force at center = newer strength
    
    EXPECT_GT(force2, force1); // Check scaling relationship
} 