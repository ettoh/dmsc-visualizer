# DMSC Visualizer
Library to visualize and solve instances of the algorithmic *Dynamic Minimum Scan Cover* Problem.

## Cloning
This repository contains submodules for external dependencies. When doing a fresh clone, make sure you clone recursively:
```
git clone --recursive <path to repository>
```
Updating an already cloned repository:
```
git submodule init
git submodule update
```


## Not included third-party libraries
* QT5 (support for OpenGL 3.3 core necessary)
* OpenGL Mathematics (GLM) - can be found here: https://github.com/g-truc/glm
* GLFW - https://github.com/glfw/glfw

## Third-party libraries (git submodules)
* Dear ImGui - https://github.com/ocornut/imgui

## Building
###  Windows specific
Ensure that the cmake environment variable ```QTDIR``` to the Qt root install location is set. Ensure that ```{QTDIR}/bin/windeployqt.exe``` exists.


## How to use
soon