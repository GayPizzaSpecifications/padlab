# PadLab #

Playground for different analogue stick setups written with SDL2 for graphics and controller support.

### Building ###
Requirements:
- C99 compiler
- SDL 3.0.0
- CMake 3.5 or higher

Optional:
- Python 3 (Only when building OpenGL Core profile or Metal backends)
- Fruit device (Only for Metal backend)

Available backends are:
- `BUILD_OPENGL_LEGACY` OpenGL Compatibility profile 1.1 (default ON)
- `BUILD_OPENGL` OpenGL Core profile 3.3 (WIP)
- `BUILD_METAL` Fruit renderer (WIP, ON by default for APPLE)

For *nix:
```shell
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_OPENGL:BOOL=ON
cmake --build build
```
