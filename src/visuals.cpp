#include "dmsc/visuals.hpp"
#include "opengl_widgets.hpp"
#include <stdio.h>
#include <stdlib.h>

namespace dmsc {

void visualizeInstance(const Instance& instance) { visualizeInstance(PhysicalInstance(instance)); }

void visualizeInstance(const PhysicalInstance& instance) {
    // TODO new thread?
    OpenGLWidget gl;
    gl.show(instance);
}

} // namespace dmsc
