# AuroraEngine Test Executable Setup

## Overview
The project now includes an `AuroraEngineTest` executable for testing the engine during local development. This executable is automatically excluded when packaging the library with Conan.

## What Was Changed

### CMakeLists.txt
1. **Added BUILD_TESTING option** (line 8): Controls whether the test executable is built
   - Defaults to `ON` for local development
   - Set to `OFF` by Conan during package creation

2. **Added AuroraEngineTest executable** (lines 41-65):
   - Compiles `game/main.cpp` and `game/player.cpp`
   - Links to the `AuroraEngine` library
   - Includes `game/` directory for `player.h`
   - Sets working directory to project root so `game/data/` paths resolve correctly

### conanfile.py
1. **Added package metadata**: name, version, license, author, description
2. **Added exports_sources**: Specifies which files to include in the package
3. **Added build() method**: Configures and builds the library with `BUILD_TESTING=OFF`
4. **Added package() method**: Copies library files and public headers to the package
5. **Added package_info() method**: Configures how consuming projects link to the library

## Usage

### Local Development
In CLion, you'll now have two build targets:
- **AuroraEngine** - The static library (builds automatically)
- **AuroraEngineTest** - The test executable

To run the test executable:
1. Select `AuroraEngineTest` from the run configurations dropdown
2. Click Run or Debug
3. The executable runs with working directory set to project root, so `game/data/` paths work correctly

### Creating a Conan Package
To release the library to your local Conan cache:

```bash
conan create . --build=missing
```

The test executable will **NOT** be included in the package. Only the library and public headers are packaged.

### Using the Package in Other Projects
In another project's `conanfile.txt` or `conanfile.py`:

```python
def requirements(self):
    self.requires("auroraengine/0.1.0")
```

Then in CMakeLists.txt:
```cmake
find_package(auroraengine REQUIRED)
target_link_libraries(YourTarget PRIVATE auroraengine::auroraengine)
```

Your project will automatically get:
- The AuroraEngine library linked
- Public headers available under `engine/` (e.g., `#include "engine/engine.h"`)
- All transitive dependencies (SDL3, SDL3_image, GLM)

## Notes
- The working directory is set via `VS_DEBUGGER_WORKING_DIRECTORY`, which works in CLion and Visual Studio
- Asset paths in `game/` code use relative paths from project root (e.g., `"game/data/GraveRobber_idle.png"`)
- Update the version in `conanfile.py` when making releases
