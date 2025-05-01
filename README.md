# Simple Particle Simulation

A C++ simulation demonstrating basic particle interactions within a containment field, focusing on parallel processing concepts and providing opportunities for debugging and optimization practice.

## Project Overview

This project simulates the behavior of particles confined within a 2D square field. Key features include:
- Particle movement and collision physics (simplified).
- A containment field applying forces to keep particles inside.
- Basic parallel processing using C++ threads and a thread manager.
- Configuration loading from a JSON file (`config.json`).
- Simple ASCII-based visualization of particle density in the terminal.
- Intentionally included bugs and areas for performance improvement for educational purposes.

## Building the Project

### Prerequisites
- C++17 compatible compiler (GCC, Clang, MSVC)
- CMake (version 3.10 or higher)
- Git (for fetching dependencies)
- OpenMP (usually included with the compiler, ensure it's enabled)

### Build Instructions
```bash
# 1. Clone the repository (if you haven't already)
# git clone <repo-url>
# cd <repo-name>

# 2. Create a build directory
mkdir build
cd build

# 3. Configure the project with CMake
cmake ..

# 4. Build the project (e.g., using Make)
make
# On Windows with Visual Studio, you might open the generated solution file
# or use: cmake --build .
```
This will create two main executables in the `build` directory:
- `quantum_simulator_app`: The main simulation application.
- `quantum_tests`: The test runner executable.
It will also copy `config.json` to the build directory.

## Running the Simulation

Navigate to the `build` directory and run the application:
```bash
./quantum_simulator_app
```
The simulation will run in the terminal, showing an ASCII representation of particle density. It uses parameters from the `config.json` file located in the same directory.

## Project Structure

```
.
├── include/               # Header files (.h)
│   ├── Config.h
│   ├── ContainmentField.h
│   ├── Particle.h
│   ├── Simulation.h
│   └── ThreadManager.h
├── src/                   # Source files (.cpp)
│   ├── ContainmentField.cpp
│   ├── main.cpp           # Application entry point
│   ├── Particle.cpp
│   ├── Simulation.cpp
│   └── ThreadManager.cpp
├── tests/                 # Unit test source files
│   ├── ContainmentTests.cpp
│   ├── ParallelizationTests.cpp
│   ├── ParticleTests.cpp
│   ├── SimulationTests.cpp
│   └── test_main.cpp      # Test runner entry point
├── CMakeLists.txt         # Build configuration script
├── config.json            # Simulation configuration file
└── README.md              # This file
```

## Test Scoring Breakdown (Total 100 Points)

The following points are assigned to each test case, reflecting its relative complexity and the importance of the functionality it covers within the simulation. Passing these tests indicates progress in fixing bugs and ensuring core features work correctly.

**`ParallelizationTests.cpp` (45 Points Total)**
*   `ParallelizationTest.ThreadCountInitialization`: 2 points (Basic setup check)
*   `ParallelizationTest.ThreadManagerActivation`: 2 points (Basic setup check)
*   `ParallelizationTest.ThreadSafeParticleAddition`: 5 points (Tests mutex usage during modification)
*   `ParallelizationTest.IsParallelized`: 10 points (Overall simulation speedup, complex interaction)
*   `ParallelizationTest.ParallelPositionUpdates`: 4 points (Benchmarking specific parallel function)
*   `ParallelizationTest.ParallelForceApplication`: 4 points (Benchmarking specific parallel function)
*   `ParallelizationTest.ParallelEnergyCalculation`: 4 points (Benchmarking specific parallel function + correctness)
*   `ParallelizationTest.ParallelCollisionHandling`: 11 points (Benchmarking complex parallel logic)
*   `ParallelizationTest.ParallelPerformance`: 3 points (Alternative overall simulation speedup check - some overlap with IsParallelized)

**`ContainmentTests.cpp` (14 Points Total)**
*   `ContainmentFieldTest.FieldSize`: 1 point (Simple getter)
*   `ContainmentFieldTest.ParticleContainment`: 4 points (Boundary logic and edge cases)
*   `ContainmentFieldTest.ContainmentForce`: 8 points (Core physics calculation logic)
*   `ContainmentFieldTest.FieldStrength`: 1 point (Simple getter/setter)

**`ParticleTests.cpp` (23 Points Total)**
*   `ParticleTest.PositionAndVelocity`: 2 points (Simple getters/setters)
*   `ParticleTest.EnergyManagement`: 4 points (Bounds checking logic)
*   `ParticleTest.Collision`: 4 points (Simplified physics interaction)
*   `ParticleTest.CollisionDetection`: 2 points (Basic distance calculation)
*   `ParticleTest.ThreadSafety`: 10 points (Concurrent access/modification, core mutex usage)
*   `ParticleTest.MemoryManagement`: 1 point (Basic object creation check)

**`SimulationTests.cpp` (18 Points Total)**
*   `SimulationTest.Initialization`: 1 point (Basic setup check)
*   `SimulationTest.AddParticle`: 2 points (Basic container modification)
*   `SimulationTest.UpdateSimulation`: 3 points (Checks if overall simulation state changes)
*   `SimulationTest.EnergyChange`: 3 points (Checks high-level energy behaviour over time)
*   `SimulationTest.ParticleInteractionAndEscape`: 6 points (Tests interaction leading to removal - involves multiple components)
*   `SimulationTest.ParallelPerformance`: 3 points (Redundant overall simulation speedup check within this suite)

**Total Score: 100 Points**

## Known Issues / Areas for Improvement

The project contains several intentional bugs and areas where performance can be improved, typical of complex simulations:

*   **Thread Safety:** Potential race conditions, deadlocks, or incorrect synchronization in particle updates, simulation steps, and thread management.
*   **Memory Management:** Potential for memory leaks or inefficient memory usage.
*   **Physics/Logic:** Possible inaccuracies in collision handling, force calculations, or energy conservation.
*   **Performance:** Opportunities to improve parallel execution speed, reduce bottlenecks, and optimize algorithms or data structures.

Good luck with debugging and optimizing the simulation!
