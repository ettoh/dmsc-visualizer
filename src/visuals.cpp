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

void visualizeDmscSolution(const Instance& instance, const DmscSolution& solution, const float t0) {
    visualizeDmscSolution(PhysicalInstance(instance), solution, t0);
}

// ------------------------------------------------------------------------------------------------

void visualizeDmscSolution(const PhysicalInstance& instance, const DmscSolution& solution, const float t0) {
    OpenGLWidget gl;
    gl.show(instance, solution, t0);
}

// ------------------------------------------------------------------------------------------------

void visualizeCustom(const Instance& instance, const Animation& animation, const float t0) {
    visualizeCustom(PhysicalInstance(instance), animation, t0);
}

// ------------------------------------------------------------------------------------------------

void visualizeCustom(const PhysicalInstance& instance, const Animation& animation, const float t0) {
    OpenGLWidget gl;
    gl.show(instance, animation, t0);
}

// ------------------------------------------------------------------------------------------------

void visualizeFreezeTagSolution(const PhysicalInstance& instance, const FreezeTagSolution& solution, const float t0) {
    OpenGLWidget gl;
    gl.show(instance, solution, t0);
}

} // namespace dmsc
