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

## What you can do with it
### Visualize satellite constellations
>see the `sample_instances` example

First we have to create a simple set of satellites (satellite constellation) moving on kaplerian orbits around the earth (or a different central mass). If you want the satellites to be able to communicate, you will have to define links between the satellites as well. 

A satellite and its movement around the earth is definded by a `StateVector` containing the six traditional orbital elements (Keplerian elements) followed by two parameters for the antenna:
```
float height_perigee;       // [km]
float eccentricity;         // [rad]
float inclination;          // [rad]
float argument_periapsis;   // [rad]
float raan;                 // [rad] 
float initial_true_anomaly; // [rad]
float rotation_speed;       // [rad/sec]
float cone_angle;           // [rad]
```

Once you created the satellite constellation and intersatellite links between them, you can visualize it by calling the `visualizeInstance` function you can find in the `visuals.hpp` header. A separate window will open and the satellite constellation will be simulated in realtime. You can change the simulation speed and other settings via the GUI.

If you defined ISLs (intersatellite links) between satellites, these will be visualized as well. The color of edges indicates whether a communication is possible or not.
 
![instance visibility](https://raw.githubusercontent.com/wiki/mc-thulu/dmsc-visualizer/web/instance_visibility.gif) ![gui overview](https://raw.githubusercontent.com/wiki/mc-thulu/dmsc-visualizer/web/gui.png)

### Visualize solutions of the **DMSC** or **Angular Freeze Tag** problem
>see the `sample_solver` and `sample_freezeTag` example

![satellite orientation change](https://raw.githubusercontent.com/wiki/mc-thulu/dmsc-visualizer/web/orientation.gif) ![freeze tag](https://raw.githubusercontent.com/wiki/mc-thulu/dmsc-visualizer/web/freeze_tag.gif)

### Visualize custom animations
>see the `sample_custom` example

![custom animation](https://raw.githubusercontent.com/wiki/mc-thulu/dmsc-visualizer/web/custom.gif)

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
