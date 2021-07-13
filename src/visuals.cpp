#include "dmsc/visuals.hpp"
#include "opengl_widgets.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace dmsc {

void visualizeInstance(const Instance& instance, const float t0) { visualizeInstance(PhysicalInstance(instance), t0); }

// ------------------------------------------------------------------------------------------------

void visualizeInstance(const PhysicalInstance& instance, const float t0) {
    OpenGLWidget gl;
    gl.show(instance, t0);
}

// ------------------------------------------------------------------------------------------------

void visualizeSolution(const Instance& instance, const Solution& solution) {
    visualizeSolution(PhysicalInstance(instance), solution);
}

// ------------------------------------------------------------------------------------------------

void visualizeSolution(const PhysicalInstance& instance, const Solution& solution) {
    OpenGLWidget gl;
    gl.show(instance, solution);
}

// ------------------------------------------------------------------------------------------------

} // namespace dmsc
