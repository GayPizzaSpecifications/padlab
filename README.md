# PadLab #

Playground for different analogue stick setups written with SDL2 for graphics and controller support.

### Building ###
Requirements:
- C99 compiler
- SDL 2.x
- CMake 3.1 or higher

Available backends are:
- `BUILD_OPENGL_LEGACY` OpenGL Compatibility profile 1.1 (default ON)
- `BUILD_OPENGL` OpenGL Core profile 3.3 (WIP)
- `BUILD_METAL` Fruit renderer (WIP, ON by default for APPLE)

OpenGL Core profile backend requires:
- Python 3

Metal backend requires:
- Fruit device
- Python 3

For *nix:
```shell
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```
