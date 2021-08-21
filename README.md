# DMSC Visualizer
A library to visualize (keplerian) orbital movements of a satellite network in 3D using [OpenGL®](https://www.khronos.org/opengl/) 4.2.

![example instance](https://raw.githubusercontent.com/wiki/mc-thulu/dmsc-visualizer/web/instance.gif)

## Download
This repository contains submodules for external dependencies. When doing a fresh clone, make sure you clone recursively:
```
git clone --recursive https://github.com/mc-thulu/dmsc-visualizer.git
```
Updating an already cloned repository:
```
git submodule init
git submodule update
```

### Not included third-party libraries
* OpenGL Mathematics (GLM) - https://github.com/g-truc/glm
* GLFW - https://github.com/glfw/glfw

### Third-party libraries (included by git submodules)
* Dear ImGui - https://github.com/ocornut/imgui
* stb - https://github.com/nothings/stb

## Build
### CMake options
* Create doxygen documentation ``-DDMSC_CREATE_DOCS=ON``
* Build examples ``-DDMSC_BUILD_SAMPLES=ON``

## How to use
soon ...

## Real world instances
The provided real world instances are based on the idealized structure of the corresponding satellite constellation. In particular, the state vectors do not correspond to a more precisely specified point in (real)time. All instances do not contain any intersatellite links.

|instance    |# satellites|orbital altitude|inclination|eccentricity|orbital planes|
|:----------:|:----------:|:--------------:|:---------:|:----------:|:------------:|
|Galileo     |30          |23.222 km (MEO) |56°        |0           |3             |
|Iridium Next|66          |780 km (LEO)    |86.4°      |0           |6             |
|OneWeb      |648         |1200 km (LEO)   |87.9°      |0           |12            |

## Credits / Attributions
* The textures used for the earth are from [NASA - Visible Earth](https://visibleearth.nasa.gov)
* OpenGL is a trademark of the [Khronos Group Inc.](http://www.khronos.org)
