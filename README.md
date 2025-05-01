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

Good luck with debugging and optimizing the simulation!
