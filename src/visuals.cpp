#include "dmsc/visuals.hpp"
#include "opengl_widgets.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

namespace dmsc {

void visualizeInstance(const Instance& instance) { visualizeInstance(PhysicalInstance(instance)); }

void visualizeInstance(const PhysicalInstance& instance) {
    // TODO new thread?
    OpenGLWidget gl = OpenGLWidget();
    gl.show(instance);
}

// ------------------------------------------------------------------------------------------------

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

} // namespace dmsc
