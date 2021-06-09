#include "dmsc/visuals.hpp"
#include "opengl_widgets.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace dmsc {

void visualizeInstance(const Instance& instance) { visualizeInstance(PhysicalInstance(instance)); }

// ------------------------------------------------------------------------------------------------

void visualizeInstance(const PhysicalInstance& instance) {
    // TODO new thread?
    OpenGLWidget gl;
    gl.show(instance);
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
